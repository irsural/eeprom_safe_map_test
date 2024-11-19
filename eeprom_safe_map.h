#ifndef NOISE_GENERATOR_EEPROM_SAFE_MAP_H
#define NOISE_GENERATOR_EEPROM_SAFE_MAP_H

#include <algorithm>
#include <array>
#include <cmath>
#include <sd_page_mem.h>
#include <vector>

/// \brief Класс для записи значений в eeprom
/// \details Записывает значения в eeprom с экономией ресурса памяти
/// \details Для корректной работы при первом использовании вызвать функцию reset
/// \param K - тип данных для ключа
/// \param V - тип данных для значения
template<class K, class V>
class eeprom_safe_map_t
{
public:
  /// \param a_free_space_bytes Кол-во свободного места в байтах
  /// \param a_data_sect_size_pages Размер сектора данных в страницах
  /// \param a_default_key Ключ, который будет использоваться по умолчанию.
  /// \param a_terminator_key Ключ, который будет терминатором списка ключей.
  /// \param a_default_value Значение, которое будет задаваться по умолчанию
  explicit eeprom_safe_map_t(sd_page_mem_t* ap_page, size_t a_free_space_bytes,
    uint32_t a_data_sect_size_pages, const K& a_default_key, const K& a_terminator_key);

  /// \brief Установить значения для выбранного ключа
  /// \details Если сектора с таким ключом нет, то такой сектор будет создан
  /// \param a_key Искомый ключ
  /// \return Если возвращается false, то закончилось место для ключей
  bool set_value(const K& a_key, V& a_value);
  bool get_value(const K& a_key, V& a_value);
  void tick();
  void add_key();
  bool ready();
  void reset();

  // Синхронные функции для дебага (после полного тестирования будут удалены)
  [[nodiscard]] uint32_t get_time_d() const;
  [[nodiscard]] uint32_t get_notes_count() const;
  void set_notes_count(uint32_t a_count);
  [[nodiscard]] std::array<uint8_t, 512> get_page(uint32_t offset = 0) const;
  [[nodiscard]] std::array<std::array<uint8_t, 512>, 16> get_sect(int32_t a_sect_number = 0) const;
  [[nodiscard]] std::array<uint8_t, 10> get_idents_index(uint32_t a_sect, uint32_t a_page) const;
  [[nodiscard]] std::array<uint8_t, 16> get_idents_index_sect(
    uint32_t a_sect, uint32_t a_value_cell) const;

private:
  enum class status_t {
    free,
    find_current_key,
    add_key,
    add_ended,
    find_current_value,
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
    write_value
  };
  enum class page_mem_op_t {
    read,
    write,
    end_op
  };

  static const uint32_t m_bytes_per_key = sizeof(K);
  static const uint32_t m_bytes_per_value = sizeof(V);
  static const uint32_t m_bytes_per_value_index = 1;
  static const uint8_t m_data_sector_default_value_byte = 0xff;

  sd_page_mem_t* mp_page;
  uint32_t m_data_sector_size_pages;
  uint32_t m_page_size;
  std::vector<uint8_t> m_page_buffer;
  const K m_terminator_key;
  K m_current_key;
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
  uint32_t m_values_per_page;
  uint32_t m_keys_per_page;
  uint32_t m_info_sector_size_pages;
  uint32_t m_data_max_sectors_count;
  std::vector<K> m_keys;
  V* mp_buf_to_save_value;
  page_mem_op_t m_page_mem_op;
  uint32_t m_page_mem_page_index;

  // Используется для дебага (после полного тестирования будет удалено)
  sd_page_mem_t* mp_debug_page;

  /// \details Ассинхронно читает и пишет в номера страниц, относительно стартовой страницы,
  /// используя внутренний буффер
  void read_page(uint32_t a_page_index, status_t a_next_status,
    add_status_t a_next_add_status = add_status_t::update_info);
  void write_page(uint32_t a_page_index, status_t a_next_status,
    add_status_t a_next_add_status = add_status_t::update_info);
  void page_mem_tick();

  void change_key(const K& a_key, action_t a_action_status);
  void evaluate_info_sector_size(uint32_t a_free_page_count);
  void get_keys();

  uint32_t get_data_sector_start_page(uint32_t a_sector);

  // Функции, которые работают с m_page_buffer
  uint8_t read_index(uint32_t a_value_cell);
  void write_index(uint32_t a_value_cell, uint8_t a_index);
  uint8_t read_value(uint32_t a_value_cell);
  void write_value(uint32_t a_value_cell, const V& a_value);
  K read_key(uint32_t a_key_index);
  void write_key(uint32_t a_key_index, const K& a_key);
};

template<class K, class V>
eeprom_safe_map_t<K, V>::eeprom_safe_map_t(sd_page_mem_t* ap_page, size_t a_free_space_bytes,
  uint32_t a_data_sect_size_pages, const K& a_default_key, const K& a_terminator_key) :
  mp_page(ap_page),
  m_data_sector_size_pages(a_data_sect_size_pages),
  m_page_size(mp_page->page_size()),
  m_page_buffer(m_page_size),
  m_terminator_key(a_terminator_key),
  m_current_key(a_default_key),
  m_current_value {},
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
  m_values_per_page(0),
  m_keys_per_page(0),
  m_info_sector_size_pages(0),
  m_data_max_sectors_count(0),
  mp_buf_to_save_value(nullptr),
  m_page_mem_op(),
  m_page_mem_page_index(0),
  mp_debug_page(
    new sd_page_mem_t(mp_page->start_page(), mp_page->page_size(), mp_page->start_page()))
{
  // Максимальный индекс должен быть на 1 больше количества страниц для работы алгоритма обнаружения
  // актуального сектора
  IRS_ASSERT(m_data_sector_size_pages < 255);
  evaluate_info_sector_size(a_free_space_bytes / m_page_size);
  get_keys();

  // Получение текущего значения
  change_key(m_current_key, action_t::none);
}

template<class K, class V>
bool eeprom_safe_map_t<K, V>::set_value(const K& a_key, V& a_value)
{
  IRS_ASSERT(ready());
  if (m_keys_count + 1 > m_info_sector_size_pages * m_keys_per_page) {
    return false;
  }
  m_new_value = a_value;
  if (m_current_key != a_key) {
    change_key(a_key, action_t::write_value);
  } else {
    m_current_value = m_new_value;
    read_page(
      get_data_sector_start_page(m_current_sector) + m_current_sector_page, status_t::write_value);
  }
  return true;
}

template<class K, class V>
bool eeprom_safe_map_t<K, V>::get_value(const K& a_key, V& a_value)
{
  IRS_ASSERT(ready());
  auto it = std::find(m_keys.begin(), m_keys.end(), a_key);
  if (it == m_keys.end()) {
    return false;
  } else {
    change_key(a_key, action_t::read_value);
    mp_buf_to_save_value = &a_value;
    return true;
  }
}

template<class K, class V>
void eeprom_safe_map_t<K, V>::tick()
{
  mp_page->tick();
  switch (m_status) {
    case status_t::free: {
    } break;

    case status_t::find_current_key: {
      auto it = std::find(m_keys.begin(), m_keys.end(), m_current_key);
      // Запись не была найдена
      if (it == m_keys.end()) {
        m_status = status_t::add_key;
        m_add_status = add_status_t::update_info;
      } else {
        // Запись была найдена
        auto key_index = std::distance(m_keys.begin(), it);
        m_current_sector = key_index % m_data_max_sectors_count;
        m_current_value_cell = key_index / m_data_max_sectors_count;
        read_page(get_data_sector_start_page(m_current_sector), status_t::find_current_value);
        m_status = status_t::find_current_value;
      }
    } break;

      // Статус добавления разделен на подстатусы
    case status_t::add_key: {
      add_key();
    } break;

    case status_t::add_ended: {
      m_current_sector_page = 0;
      m_current_value_index = 0;
      m_current_value = m_new_value;
      read_page(get_data_sector_start_page(m_current_sector), status_t::write_value);
    } break;

      // Поиск последней записи значения
    case status_t::find_current_value: {
      uint8_t value_index = read_index(m_current_value_cell);
      bool no_jump = ident_index == (m_current_sector_index + 1) % (m_data_sector_size_pages + 1);
      bool has_value = ident_index != m_data_sector_default_value_byte;
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
            m_current_value = m_new_value;
            read_page(get_data_sector_start_page(m_current_sector) + m_current_sector_page,
              status_t::write_value);
          } break;
        }
        m_action_status = action_t::none;
      }
    } break;

      // Запись новой ячейки значения в следующую страницу сектора
    case status_t::write_value: {
      write_value(m_current_value_cell, m_current_value);
      write_index(m_current_value_cell, m_current_value_index);
      write_page(
        get_data_sector_start_page(m_current_sector) + m_current_sector_page, status_t::free);
      m_current_value_index = (m_current_value_index + 1) % (m_data_sector_size_pages + 1);
      m_current_sector_page = (m_current_sector_page + 1) % m_data_sector_size_pages;
    } break;

    case status_t::wait_page_mem: {
      page_mem_tick();
    } break;
  }
}

template<class K, class V>
void eeprom_safe_map_t<K, V>::add_key()
{
  switch (m_add_status) {
    case add_status_t::free: {
    } break;

    case add_status_t::update_info: {
      m_keys.emplace_back(m_current_key);
      m_keys_count++;
      m_current_sector = (m_keys_count - 1) % m_data_max_sectors_count;
      m_current_value_cell = (m_keys_count - 1) / m_data_max_sectors_count;

      // Если места под новый сектор нет, то значение будет храниться в уже существующем
      if (m_keys_count > m_data_max_sectors_count) {
        m_add_status = add_status_t::add_key_prep;
      } else {
        // Добавление сектора, заполнение его значением m_data_sector_default_value_byte
        for (uint32_t i = 0; i < m_page_size; ++i) {
          m_page_buffer[i] = m_data_sector_default_value_byte;
        }
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

      // Копируется страница, в которую будет добавлена запись
    case add_status_t::add_key_prep: {
      // -1 для перевода из кол-ва в индекс
      m_current_sector_page = (m_keys_count - 1) / m_keys_per_page;
      read_page(m_current_sector_page, status_t::add_key, add_status_t::add_key);
    } break;

      // Вставка измененной страницы, которая была взята на предыдущем шаге
    case add_status_t::add_key: {
      // m_keys_count - m_notes_per_page * m_current_sector_page - 1 = номер
      // записи в текущей странице
      write_key(m_keys_count - m_keys_per_page * m_current_sector_page - 1, m_current_key);
      // Проверка на наличие места для ключа-терминатора в текущей странице
      if (m_keys_count / m_keys_per_page == m_current_sector_page) {
        write_key(m_keys_count - m_keys_per_page * m_current_sector_page, m_terminator_key);
        write_page(m_current_sector_page, status_t::add_ended);
      } else {
        write_page(m_current_sector_page, status_t::add_key, add_status_t::add_terminator_key);
      }
    } break;

      // Добавление символа-терминатора в конец, если он не был добавлен в предыдущем состоянии
    case add_status_t::add_terminator_key: {
      IRS_ASSERT(m_current_sector_page + 1 < m_info_sector_size_pages);
      *reinterpret_cast<K*>(m_page_buffer.data()) = m_terminator_key;
      write_page(m_current_sector_page + 1, status_t::add_ended);
    } break;
  }
}

template<class K, class V>
bool eeprom_safe_map_t<K, V>::ready()
{
  return m_status == status_t::free;
}

template<class K, class V>
void eeprom_safe_map_t<K, V>::read_page(
  uint32_t a_page_index, status_t a_next_status, add_status_t a_next_add_status)
{
  m_page_mem_page_index = a_page_index;
  m_page_mem_op = page_mem_op_t::read;
  m_status = status_t::wait_page_mem;
  m_next_status = a_next_status;
  m_next_add_status = a_next_add_status;
}

template<class K, class V>
void eeprom_safe_map_t<K, V>::write_page(
  uint32_t a_page_index, status_t a_next_status, add_status_t a_next_add_status)
{
  m_page_mem_page_index = a_page_index;
  m_page_mem_op = page_mem_op_t::write;
  m_status = status_t::wait_page_mem;
  m_next_status = a_next_status;
  m_next_add_status = a_next_add_status;
}

template<class K, class V>
void eeprom_safe_map_t<K, V>::page_mem_tick()
{
  if (mp_page->ready()) {
    switch (m_page_mem_op) {
      case page_mem_op_t::free: {
      } break;
      case page_mem_op_t::read: {
        mp_page->read_page(m_page_buffer.data(), m_page_mem_page_index);
        m_page_mem_op = page_mem_op_t::wait;
      } break;
      case page_mem_op_t::write: {
        mp_page->write_page(m_page_buffer.data(), m_page_mem_page_index);
        m_page_mem_op = page_mem_op_t::wait;
      } break;
      case page_mem_op_t::end_op: {
        m_status = m_next_status;
        m_add_status = m_next_add_status;
      } break;
    }
  }
}

template<class K, class V>
void eeprom_safe_map_t<K, V>::change_key(const K& a_key, action_t a_action_status)
{
  m_status = status_t::find_current_key;
  m_current_key = a_key;
  m_current_sector_page = 0;
  m_current_value_index = 0;
  m_action_status = a_action_status;
}

template<class K, class V>
void eeprom_safe_map_t<K, V>::reset()
{
  *reinterpret_cast<K*>(m_page_buffer.data()) = m_terminator_key;
  while (!mp_page->ready()) {
    mp_page->tick();
  }
  // Запись символа-терминатора на первую позицию для ключей
  mp_page->write_page(m_page_buffer.data(), 0);
  while (!mp_page->ready()) {
    mp_page->tick();
  }
  m_keys_count = 0;
  m_keys.clear();
  change_key(m_current_key, action_t::write_value);
}

template<class K, class V>
void eeprom_safe_map_t<K, V>::evaluate_info_sector_size(uint32_t a_free_page_count)
{
  m_keys_per_page = m_page_size / m_bytes_per_key;
  m_values_per_page = m_page_size / (m_bytes_per_value + m_bytes_per_value_index);
  // p_vk = values_per_page / keys_per_page - кол-во страниц ключей для хранения ячеек значений,
  // которые помещаются на одной странице (Если на странице помещается 20 ячеек значений, а ключей
  // только 8, то потребуется 2,5 страницы с ключами, чтобы хранить 20 ячеек значений)
  float key_pages_per_value_page =
    static_cast<float>(m_values_per_page) / static_cast<float>(m_keys_per_page);
  // Для каждого сектора необходимо иметь p_vk страниц с ключами,
  // сектор занимает data_sect_size_pages страниц. При оптимальном распределении получается
  // уравнение: (p_vk + data_sect_size_pages) * k = p, где k - кол-во пар "страниц записи - сектор"
  // или кол-во секторов данных. Выражается k: k = floor(p / (p_vk + data_sect_size_pages)).
  m_data_max_sectors_count = static_cast<uint32_t>(static_cast<float>(a_free_page_count) /
    (key_pages_per_value_page + static_cast<float>(m_data_sector_size_pages)));
  m_info_sector_size_pages = static_cast<uint32_t>(
    ceil(static_cast<float>(m_data_max_sectors_count) * key_pages_per_value_page));
}

template<class K, class V>
void eeprom_safe_map_t<K, V>::get_keys()
{
  while (!mp_page->ready()) {
    mp_page->tick();
  }
  for (size_t i = 0; i < m_info_sector_size_pages; ++i) {
    mp_page->read_page(m_page_buffer.data(), i);
    while (!mp_page->ready()) {
      mp_page->tick();
    }
    bool key_terminated_value_found = false;
    for (size_t j = 0; j < m_keys_per_page; ++j) {
      K tmp_value = read_key(j);
      // Если найдено значение m_terminator_key, то это конец списка ключей
      if (tmp_value == m_terminator_key && (i || j)) {
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
uint32_t eeprom_safe_map_t<K, V>::get_data_sector_start_page(uint32_t a_sector)
{
  return m_info_sector_size_pages + a_sector * m_data_sector_size_pages;
}

template<class K, class V>
uint8_t eeprom_safe_map_t<K, V>::read_index(uint32_t a_value_cell)
{
  return *reinterpret_cast<uint8_t*>(
    m_page_buffer.data() + m_page_size - m_bytes_per_value_index * (a_value_cell + 1));
}

template<class K, class V>
void eeprom_safe_map_t<K, V>::write_index(uint32_t a_value_cell, uint8_t a_index)
{
  *reinterpret_cast<uint8_t*>(
    m_page_buffer.data() + m_page_size - m_bytes_per_value_index * (a_value_cell + 1)) = a_index;
}

template<class K, class V>
uint8_t eeprom_safe_map_t<K, V>::read_value(uint32_t a_value_cell)
{
  return *reinterpret_cast<V*>(m_page_buffer.data() + a_value_cell * m_bytes_per_value);
}

template<class K, class V>
void eeprom_safe_map_t<K, V>::write_value(uint32_t a_value_cell, const V& a_value)
{
  *reinterpret_cast<uint32_t*>(m_page_buffer.data() + a_value_cell * m_bytes_per_value) = a_value;
}

template<class K, class V>
K eeprom_safe_map_t<K, V>::read_key(uint32_t a_key_index)
{
  return *reinterpret_cast<K*>(m_page_buffer.data() + a_key_index * m_bytes_per_key);
}

template<class K, class V>
void eeprom_safe_map_t<K, V>::write_key(uint32_t a_key_index, const K& a_key)
{
  *reinterpret_cast<K*>(m_page_buffer.data() + a_key_index * m_bytes_per_key) = a_key;
}

template<class K, class V>
uint32_t eeprom_safe_map_t<K, V>::get_time_d() const
{
  std::array<uint8_t, 512> tmp {};
  uint32_t time, max_time = 0;
  for (uint32_t i = 0; i < 16; ++i) {
    mp_debug_page->read_page(tmp.data(), m_info_sector_size_pages + m_current_sector + i);
    mp_debug_page->tick();
    time = *reinterpret_cast<uint32_t*>(tmp.data());
    if (time > max_time) {
      max_time = time;
    }
  }
  return max_time;
}

template<class K, class V>
uint32_t eeprom_safe_map_t<K, V>::get_notes_count() const
{
  std::array<uint8_t, 512> tmp {};
  mp_debug_page->read_page(tmp.data(), 0);
  mp_debug_page->tick();
  return *reinterpret_cast<uint32_t*>(tmp.data());
}

template<class K, class V>
void eeprom_safe_map_t<K, V>::set_notes_count(uint32_t a_count)
{
  std::array<uint8_t, 512> tmp {};
  *reinterpret_cast<uint32_t*>(tmp.data()) = a_count;
  mp_debug_page->write_page(tmp.data(), 0);
  mp_debug_page->tick();
  m_keys_count = a_count;
}

template<class K, class V>
std::array<uint8_t, 512> eeprom_safe_map_t<K, V>::get_page(uint32_t offset) const
{
  std::array<uint8_t, 512> page {};
  while (!mp_debug_page->ready()) {
    mp_debug_page->tick();
    mp_page->tick();
  }
  mp_debug_page->read_page(page.data(), offset);
  while (!mp_debug_page->ready()) {
    mp_debug_page->tick();
  }
  return page;
}

template<class K, class V>
std::array<std::array<uint8_t, 512>, 16> eeprom_safe_map_t<K, V>::get_sect(
  int32_t a_sect_number) const
{
  if (a_sect_number == -1) {
    a_sect_number = static_cast<int32_t>(m_current_sector);
  }
  while (!mp_debug_page->ready()) {
    mp_debug_page->tick();
    mp_page->tick();
  }
  std::array<std::array<uint8_t, 512>, 16> sect {};
  for (uint32_t i = 0; i < m_data_sector_size_pages; ++i) {
    mp_debug_page->read_page(
      sect[i].data(), m_info_sector_size_pages + a_sect_number * m_data_sector_size_pages + i);
    while (!mp_debug_page->ready()) {
      mp_debug_page->tick();
    }
  }
  return sect;
}

template<class K, class V>
std::array<uint8_t, 10> eeprom_safe_map_t<K, V>::get_idents_index(
  uint32_t a_sect, uint32_t a_page) const
{
  std::array<uint8_t, 512> page {};
  mp_debug_page->read_page(
    page.data(), m_info_sector_size_pages + a_sect * m_data_sector_size_pages + a_page);
  mp_debug_page->tick();
  return *reinterpret_cast<std::array<uint8_t, 10>*>(page.data() + m_page_size - 10);
}

template<class K, class V>
std::array<uint8_t, 16> eeprom_safe_map_t<K, V>::get_idents_index_sect(
  uint32_t a_sect, uint32_t a_value_cell) const
{
  std::array<uint8_t, 512> page {};
  std::array<uint8_t, 16> result {};
  for (size_t i = 0; i < 16; ++i) {
    mp_debug_page->read_page(
      page.data(), m_info_sector_size_pages + a_sect * m_data_sector_size_pages + i);
    mp_debug_page->tick();
    result[i] = *reinterpret_cast<uint8_t*>(
      page.data() + m_page_size - m_bytes_per_value_index * (a_value_cell + 1));
  }
  return result;
}

#endif // NOISE_GENERATOR_EEPROM_SAFE_MAP_H
