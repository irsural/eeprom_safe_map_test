#ifndef NOISE_GENERATOR_EEPROM_SAFE_MAP_H
#define NOISE_GENERATOR_EEPROM_SAFE_MAP_H

#include <algorithm>
#include <array>
#include <cmath>
#include <irsmem.h>
#include <vector>

/// \brief ����� ��� ������ �������� � eeprom
/// \details ���������� �������� � eeprom � ��������� ������� ������
/// \details ��� ���������� ������ ��� ������ ������������� ������� ������� reset
/// \param K - ��� ������ ��� �����
/// \param V - ��� ������ ��� ��������
template<class K, class V>
class eeprom_safe_map_t {
public:
    /// \param a_page_offset ��������, � ������� �������� ������ � eeprom
    /// \param a_free_pages ���-�� ��������� �������
    /// \param a_data_sect_size_pages ������ ������� ������ � ���������
    /// \param a_default_key ����, ������� ����� �������������� �� ���������.
    /// \param a_terminator_key ����, ������� ����� ������������ ������ ������.
    explicit eeprom_safe_map_t(irs::page_mem_t *ap_page, uint32_t a_page_offset, size_t a_free_pages,
                               uint32_t a_data_sect_size_pages, const K &a_default_key, const K &a_terminator_key);

    /// \brief ���������� �������� ��� ���������� �����
    /// \details ���� ������� � ����� ������ ���, �� ����� ������ ����� ������
    /// \param a_key ������� ����
    /// \return ���� ������������ false, �� ����������� ����� ��� ������
    bool set_value(const K &a_key, V &a_value);
    bool get_value(const K &a_key, V &a_value);
    /// \brief �������� ���� a_old_key �� a_new_key � ����� ��������� a_value
    /// \details ���� ������ ���� �� ��� ������������ ����, �� ��� ������� ���������� ������� set_value
    void replace_key(const K &a_old_key, const K &a_new_key, V &a_value);
    void tick();
    void add_key();
    bool ready();
    void reset();
    [[nodiscard]] uint32_t get_keys_count() const;
    [[nodiscard]] K get_key(uint32_t a_index) const;

private:
    enum class status_t {
        free,
        find_current_key,
        add_key,
        add_ended,
        find_current_value,
        override_key,
        wait_override,
        write_value,
        wait_page_mem
    };
    enum class add_status_t {
        update_info,
        add_data_sector,
        add_key_prep,
        add_key,
        add_terminator_key
    };
    enum class action_t {
        none,
        read_value,
        write_value,
        replace_key
    };
    enum class page_mem_op_t {
        read,
        write,
        end_op
    };

    static const uint32_t m_bytes_per_key = sizeof(K);
    static const uint32_t m_bytes_per_value = sizeof(V);
    static const uint32_t m_bytes_per_value_index = 1;
    const uint8_t m_data_sector_default_value_byte = 0xff;

    irs::page_mem_t *mp_page;
    uint32_t m_data_sector_size_pages;
    uint32_t m_page_size;
    std::vector<uint8_t> m_page_buffer;
    const K m_terminator_key;
    K m_current_key;
    K m_new_key;
    V m_current_value;
    V m_new_value;
    uint32_t m_current_sector;
    uint32_t m_current_sector_page;
    uint8_t m_current_value_index;
    uint32_t m_current_value_cell;
    status_t m_status;
    status_t m_next_status;
    add_status_t m_add_status;
    add_status_t m_next_add_status;
    action_t m_action_status;
    uint32_t m_keys_count;
    uint32_t m_max_keys_count;
    uint32_t m_values_per_page;
    uint32_t m_keys_per_page;
    uint32_t m_info_sector_size_pages;
    uint32_t m_data_max_sectors_count;
    std::vector<K> m_keys;
    uint32_t m_current_key_index;
    V *mp_buf_to_save_value;
    page_mem_op_t m_page_mem_op;
    uint32_t m_page_mem_page_index;
    uint32_t m_page_offset;

    /// \details ����������� ������ � ����� � ������ �������, ������������ ��������� ��������,
    /// ��������� ���������� ������
    void read_page(uint32_t a_page_index, status_t a_next_status,
                   add_status_t a_next_add_status = add_status_t::update_info);
    void write_page(uint32_t a_page_index, status_t a_next_status,
                    add_status_t a_next_add_status = add_status_t::update_info);
    void page_mem_tick();

    void change_key(const K &a_key, action_t a_action_status);
    void evaluate_info_sector_size(uint32_t a_free_page_count);
    void get_keys();

    uint32_t get_data_sector_start_page(uint32_t a_sector);

    // �������, ������� �������� � m_page_buffer
    uint8_t read_index(uint32_t a_value_cell);
    void write_index(uint32_t a_value_cell, uint8_t a_index);
    uint8_t read_value(uint32_t a_value_cell);
    void write_value(uint32_t a_value_cell, const V &a_value);
    K read_key(uint32_t a_key_index);
    void write_key(uint32_t a_key_index, const K &a_key);
    void clear_page_buffer();
    bool is_page_ready();
};

template<class K, class V>
eeprom_safe_map_t<K, V>::eeprom_safe_map_t(irs::page_mem_t *ap_page, uint32_t a_page_offset,
                                           size_t a_free_pages, uint32_t a_data_sect_size_pages, const K &a_default_key,
                                           const K &a_terminator_key) : mp_page(ap_page),
                                                                        m_data_sector_size_pages(a_data_sect_size_pages),
                                                                        m_page_size(mp_page->page_size()),
                                                                        m_page_buffer(m_page_size),
                                                                        m_terminator_key(a_terminator_key),
                                                                        m_current_key(a_default_key),
                                                                        m_new_key(a_default_key),
                                                                        m_current_value{},
                                                                        m_new_value(),
                                                                        m_current_sector(0),
                                                                        m_current_sector_page(0),
                                                                        m_current_value_index(0),
                                                                        m_current_value_cell(0),
                                                                        m_status(status_t::free),
                                                                        m_next_status(status_t::free),
                                                                        m_add_status(),
                                                                        m_next_add_status(),
                                                                        m_action_status(),
                                                                        m_keys_count(0),
                                                                        m_max_keys_count(0),
                                                                        m_values_per_page(0),
                                                                        m_keys_per_page(0),
                                                                        m_info_sector_size_pages(0),
                                                                        m_data_max_sectors_count(0),
                                                                        m_current_key_index(0),
                                                                        mp_buf_to_save_value(nullptr),
                                                                        m_page_mem_op(),
                                                                        m_page_mem_page_index(0),
                                                                        m_page_offset(a_page_offset) {
    // ������������ ������ ������ ���� �� 1 ������ ���������� ������� ��� ������ ��������� �����������
    // ����������� �������
    IRS_ASSERT(m_data_sector_size_pages < 255);
    clear_page_buffer();
    evaluate_info_sector_size(a_free_pages);
    get_keys();

    // ��������� �������� ��������
    change_key(m_current_key, action_t::none);
}

template<class K, class V>
bool eeprom_safe_map_t<K, V>::set_value(const K &a_key, V &a_value) {
    IRS_ASSERT(ready());
    if (std::find(m_keys.begin(), m_keys.end(), a_key) == m_keys.end() && m_keys_count + 1 > m_max_keys_count) {
        return false;
    }
    m_new_value = a_value;
    if (m_current_key != a_key) {
        change_key(a_key, action_t::write_value);
    } else {
        read_page(
                get_data_sector_start_page(m_current_sector) + m_current_sector_page, status_t::write_value);
    }
    return true;
}

template<class K, class V>
bool eeprom_safe_map_t<K, V>::get_value(const K &a_key, V &a_value) {
    IRS_ASSERT(ready());
    if (std::find(m_keys.begin(), m_keys.end(), a_key) == m_keys.end()) {
        return false;
    } else {
        change_key(a_key, action_t::read_value);
        mp_buf_to_save_value = &a_value;
        return true;
    }
}

template<class K, class V>
void eeprom_safe_map_t<K, V>::replace_key(const K &a_old_key, const K &a_new_key, V &a_value) {
    IRS_ASSERT(ready());
    if (std::find(m_keys.begin(), m_keys.end(), a_new_key) != m_keys.end()) {
        set_value(a_new_key, a_value);
    } else {
        m_new_key = a_new_key;
        m_new_value = a_value;
        change_key(a_old_key, action_t::replace_key);
    }
}

template<class K, class V>
void eeprom_safe_map_t<K, V>::tick() {
    mp_page->tick();
    switch (m_status) {
        case status_t::free: {
        } break;

        case status_t::find_current_key: {
            auto it = std::find(m_keys.begin(), m_keys.end(), m_current_key);
            // ������ �� ���� �������
            if (it == m_keys.end()) {
                m_status = status_t::add_key;
                m_add_status = add_status_t::update_info;
                m_current_key_index = m_keys_count;
            } else {
                // ������ ���� �������
                m_current_key_index = std::distance(m_keys.begin(), it);
                m_current_sector = m_current_key_index % m_data_max_sectors_count;
                m_current_value_cell = m_current_key_index / m_data_max_sectors_count;
                read_page(get_data_sector_start_page(m_current_sector), status_t::find_current_value);
            }
        } break;

            // ������ ���������� �������� �� ����������
        case status_t::add_key: {
            add_key();
        } break;

        case status_t::add_ended: {
            m_current_sector_page = 0;
            m_current_value_index = 0;
            read_page(get_data_sector_start_page(m_current_sector), status_t::write_value);
        } break;

            // ����� ��������� ������ ��������
        case status_t::find_current_value: {
            uint8_t value_index = read_index(m_current_value_cell);
            bool no_jump = value_index == (m_current_value_index + 1) % (m_data_sector_size_pages + 1);
            bool has_value = value_index != m_data_sector_default_value_byte;
            bool in_range = m_current_sector_page < m_data_sector_size_pages;
            if ((m_current_sector_page == 0 || (in_range && no_jump)) && has_value) {
                m_current_value_index = value_index;
                m_current_value = read_value(m_current_value_cell);
                m_current_sector_page++;
                if (m_current_sector_page < m_data_sector_size_pages) {
                    read_page(get_data_sector_start_page(m_current_sector) + m_current_sector_page,
                              status_t::find_current_value);
                }
            } else {
                if (m_current_sector_page == 0) {
                    m_current_value_index = 0;
                } else {
                    m_current_value_index = (m_current_value_index + 1) % (m_data_sector_size_pages + 1);
                    if (m_current_sector_page == m_data_sector_size_pages) {
                        m_current_sector_page = 0;
                    }
                }
                switch (m_action_status) {
                    case action_t::none: {
                        m_status = status_t::free;
                    } break;
                    case action_t::read_value: {
                        IRS_ASSERT(mp_buf_to_save_value != nullptr);
                        *mp_buf_to_save_value = m_current_value;
                        mp_buf_to_save_value = nullptr;
                        m_status = status_t::free;
                    } break;
                    case action_t::write_value: {
                        read_page(get_data_sector_start_page(m_current_sector) + m_current_sector_page,
                                  status_t::write_value);
                    } break;
                    case action_t::replace_key: {
                        read_page(m_current_key_index / m_keys_per_page, status_t::override_key);
                    } break;
                }
                m_action_status = action_t::none;
            }
        } break;

        case status_t::override_key: {
            write_key(m_current_key_index % m_keys_per_page, m_new_key);
            write_page(m_current_key_index / m_keys_per_page, status_t::wait_override);
        } break;

        case status_t::wait_override: {
            read_page(get_data_sector_start_page(m_current_sector) + m_current_sector_page,
                      status_t::write_value);
        } break;

            // ������ ����� ������ �������� � ��������� �������� �������
        case status_t::write_value: {
            write_value(m_current_value_cell, m_new_value);
            write_index(m_current_value_cell, m_current_value_index);
            write_page(
                    get_data_sector_start_page(m_current_sector) + m_current_sector_page, status_t::free);
            m_current_value_index = (m_current_value_index + 1) % (m_data_sector_size_pages + 1);
            m_current_sector_page = (m_current_sector_page + 1) % m_data_sector_size_pages;
            m_current_value = m_new_value;
        } break;

        case status_t::wait_page_mem: {
            page_mem_tick();
        } break;
    }
}

template<class K, class V>
void eeprom_safe_map_t<K, V>::add_key() {
    switch (m_add_status) {
        case add_status_t::update_info: {
            m_keys.emplace_back(m_current_key);
            m_keys_count++;
            m_current_sector = (m_keys_count - 1) % m_data_max_sectors_count;
            m_current_value_cell = (m_keys_count - 1) / m_data_max_sectors_count;

            // ���� ����� ��� ����� ������ ���, �� �������� ����� ��������� � ��� ������������
            if (m_keys_count > m_data_max_sectors_count) {
                m_add_status = add_status_t::add_key_prep;
            } else {
                // ���������� �������, ���������� ��� ��������� m_data_sector_default_value_byte
                clear_page_buffer();
                m_current_sector_page = 0;
                m_add_status = add_status_t::add_data_sector;
            }
        } break;

        case add_status_t::add_data_sector: {
            if (m_current_sector_page < m_data_sector_size_pages - 1) {
                write_page(get_data_sector_start_page(m_current_sector) + m_current_sector_page,
                           status_t::add_key, add_status_t::add_data_sector);
                m_current_sector_page++;
            } else {
                write_page(get_data_sector_start_page(m_current_sector) + m_current_sector_page,
                           status_t::add_key, add_status_t::add_key_prep);
            }
        } break;

            // ���������� ��������, � ������� ����� ��������� ������
        case add_status_t::add_key_prep: {
            // -1 ��� �������� �� ���-�� � ������
            m_current_sector_page = (m_keys_count - 1) / m_keys_per_page;
            read_page(m_current_sector_page, status_t::add_key, add_status_t::add_key);
        } break;

            // ������� ���������� ��������, ������� ���� ����� �� ���������� ����
        case add_status_t::add_key: {
            // m_keys_count - m_notes_per_page * m_current_sector_page - 1 = �����
            // ������ � ������� ��������
            write_key(m_keys_count - m_keys_per_page * m_current_sector_page - 1, m_current_key);
            // �������� �� ������� ����� ��� �����-����������� � ������� ��������
            if (m_keys_count / m_keys_per_page == m_current_sector_page) {
                write_key(m_keys_count - m_keys_per_page * m_current_sector_page, m_terminator_key);
                write_page(m_current_sector_page, status_t::add_ended);
            } else {
                write_page(m_current_sector_page, status_t::add_key, add_status_t::add_terminator_key);
            }
        } break;

            // ���������� �������-����������� � �����, ���� �� �� ��� �������� � ���������� ���������
        case add_status_t::add_terminator_key: {
            IRS_ASSERT(m_current_sector_page + 1 < m_info_sector_size_pages);
            clear_page_buffer();
            write_key(0, m_terminator_key);
            write_page(m_current_sector_page + 1, status_t::add_ended);
        } break;
    }
}

template<class K, class V>
bool eeprom_safe_map_t<K, V>::ready() {
    return m_status == status_t::free;
}

template<class K, class V>
void eeprom_safe_map_t<K, V>::read_page(
        uint32_t a_page_index, status_t a_next_status, add_status_t a_next_add_status) {
    m_page_mem_page_index = a_page_index;
    m_page_mem_op = page_mem_op_t::read;
    m_status = status_t::wait_page_mem;
    m_next_status = a_next_status;
    m_next_add_status = a_next_add_status;
}

template<class K, class V>
void eeprom_safe_map_t<K, V>::write_page(
        uint32_t a_page_index, status_t a_next_status, add_status_t a_next_add_status) {
    m_page_mem_page_index = a_page_index;
    m_page_mem_op = page_mem_op_t::write;
    m_status = status_t::wait_page_mem;
    m_next_status = a_next_status;
    m_next_add_status = a_next_add_status;
}

template<class K, class V>
void eeprom_safe_map_t<K, V>::page_mem_tick() {
    if (is_page_ready()) {
        switch (m_page_mem_op) {
            case page_mem_op_t::read: {
                mp_page->read_page(m_page_buffer.data(), m_page_offset + m_page_mem_page_index);
                m_page_mem_op = page_mem_op_t::end_op;
            } break;
            case page_mem_op_t::write: {
                mp_page->write_page(m_page_buffer.data(), m_page_offset + m_page_mem_page_index);
                m_page_mem_op = page_mem_op_t::end_op;
            } break;
            case page_mem_op_t::end_op: {
                m_status = m_next_status;
                m_add_status = m_next_add_status;
            } break;
        }
    }
}

template<class K, class V>
void eeprom_safe_map_t<K, V>::change_key(const K &a_key, action_t a_action_status) {
    m_status = status_t::find_current_key;
    m_current_key = a_key;
    m_current_sector_page = 0;
    m_current_value_index = 0;
    m_action_status = a_action_status;
}

template<class K, class V>
void eeprom_safe_map_t<K, V>::reset() {
    clear_page_buffer();
    write_key(0, m_terminator_key);
    while (!is_page_ready()) {
        mp_page->tick();
    }
    // ������ �������-����������� �� ������ ������� ��� ������
    mp_page->write_page(m_page_buffer.data(), m_page_offset);
    while (!is_page_ready()) {
        mp_page->tick();
    }
    m_keys_count = 0;
    m_keys.clear();
    change_key(m_current_key, action_t::write_value);
}

template<class K, class V>
uint32_t eeprom_safe_map_t<K, V>::get_keys_count() const
{
    return m_keys_count;
}

template<class K, class V>
K eeprom_safe_map_t<K, V>::get_key(uint32_t a_index) const
{
    return m_keys[a_index];
}

template<class K, class V>
void eeprom_safe_map_t<K, V>::evaluate_info_sector_size(uint32_t a_free_page_count) {
    m_keys_per_page = m_page_size / m_bytes_per_key;
    m_values_per_page = m_page_size / (m_bytes_per_value + m_bytes_per_value_index);
    // p_vk = values_per_page / keys_per_page - ���-�� ������� ������ ��� �������� ����� ��������,
    // ������� ���������� �� ����� �������� (���� �� �������� ���������� 20 ����� ��������, � ������
    // ������ 8, �� ����������� 2,5 �������� � �������, ����� ������� 20 ����� ��������)
    float key_pages_per_value_page =
            static_cast<float>(m_values_per_page) / static_cast<float>(m_keys_per_page);
    // ��� ������� ������� ���������� ����� p_vk ������� � �������,
    // ������ �������� data_sect_size_pages �������. ��� ����������� ������������� ����������
    // ���������: (p_vk + data_sect_size_pages) * k = p, ��� k - ���-�� ��� "������� ������ - ������"
    // ��� ���-�� �������� ������. ���������� k: k = floor(p / (p_vk + data_sect_size_pages)).
    m_data_max_sectors_count = static_cast<uint32_t>(static_cast<float>(a_free_page_count) /
                                                     (key_pages_per_value_page + static_cast<float>(m_data_sector_size_pages)));
    m_info_sector_size_pages = static_cast<uint32_t>(
            ceil(static_cast<float>(m_data_max_sectors_count) * key_pages_per_value_page));
    m_max_keys_count = m_data_max_sectors_count * m_values_per_page;
}

template<class K, class V>
void eeprom_safe_map_t<K, V>::get_keys() {
    while (!is_page_ready()) {
        mp_page->tick();
    }
    for (size_t i = 0; i < m_info_sector_size_pages; ++i) {
        mp_page->read_page(m_page_buffer.data(), m_page_offset + i);
        while (!is_page_ready()) {
            mp_page->tick();
        }
        bool key_terminated_value_found = false;
        for (size_t j = 0; j < m_keys_per_page; ++j) {
            K tmp_value = read_key(j);
            // ���� ������� �������� m_terminator_key, �� ��� ����� ������ ������
            if (tmp_value == m_terminator_key) {
                key_terminated_value_found = true;
                break;
            }
            m_keys.emplace_back(tmp_value);
        }
        if (key_terminated_value_found) {
            break;
        }
    }
    m_keys_count = m_keys.size();
}

template<class K, class V>
uint32_t eeprom_safe_map_t<K, V>::get_data_sector_start_page(uint32_t a_sector) {
    return m_info_sector_size_pages + a_sector * m_data_sector_size_pages;
}

template<class K, class V>
uint8_t eeprom_safe_map_t<K, V>::read_index(uint32_t a_value_cell) {
    return *reinterpret_cast<uint8_t *>(
            m_page_buffer.data() + m_page_size - m_bytes_per_value_index * (a_value_cell + 1));
}

template<class K, class V>
void eeprom_safe_map_t<K, V>::write_index(uint32_t a_value_cell, uint8_t a_index) {
    *reinterpret_cast<uint8_t *>(
            m_page_buffer.data() + m_page_size - m_bytes_per_value_index * (a_value_cell + 1)) = a_index;
}

template<class K, class V>
uint8_t eeprom_safe_map_t<K, V>::read_value(uint32_t a_value_cell) {
    return *reinterpret_cast<V *>(m_page_buffer.data() + a_value_cell * m_bytes_per_value);
}

template<class K, class V>
void eeprom_safe_map_t<K, V>::write_value(uint32_t a_value_cell, const V &a_value) {
    *reinterpret_cast<uint32_t *>(m_page_buffer.data() + a_value_cell * m_bytes_per_value) = a_value;
}

template<class K, class V>
K eeprom_safe_map_t<K, V>::read_key(uint32_t a_key_index) {
    return *reinterpret_cast<K *>(m_page_buffer.data() + a_key_index * m_bytes_per_key);
}

template<class K, class V>
void eeprom_safe_map_t<K, V>::write_key(uint32_t a_key_index, const K &a_key) {
    *reinterpret_cast<K *>(m_page_buffer.data() + a_key_index * m_bytes_per_key) = a_key;
}

template<class K, class V>
void eeprom_safe_map_t<K, V>::clear_page_buffer() {
    std::fill(m_page_buffer.begin(), m_page_buffer.end(), m_data_sector_default_value_byte);
}

template<class K, class V>
bool eeprom_safe_map_t<K, V>::is_page_ready() {
    return mp_page->status() == irs_st_ready;
}

#endif// NOISE_GENERATOR_EEPROM_SAFE_MAP_H
