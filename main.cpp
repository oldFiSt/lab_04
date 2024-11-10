#include <iostream>
#include <map>
#include <vector>

template <typename T>
class CustomAllocator {
public:
    using value_type = T;
    using pointer = T*;
    using const_pointer = const T*;
    using reference = T&;
    using const_reference = const T&;
    using size_type = std::size_t;
    using difference_type = std::ptrdiff_t;

    // Конструктор без аргументов
    CustomAllocator() noexcept {}

    // Конструктор копирования
    CustomAllocator(const CustomAllocator& other) noexcept {}

    // Оператор присваивания
    CustomAllocator& operator=(const CustomAllocator& other) noexcept {
        return *this;
    }

    // Выделение памяти
    pointer allocate(size_type n) {
        if (n == 0) {
            return nullptr;
        }
        // Использование `new` для выделения памяти.
        return static_cast<pointer>(operator new(n * sizeof(T)));
    }

    // Освобождение памяти
    void deallocate(pointer p, size_type n) {
        operator delete(p);
    }

    // Макрос `std::allocator_traits` для проверки соответствия требованиям стандартного аллокатора.
    template <typename U>
    struct rebind {
        using other = CustomAllocator<U>;
    };

    void construct(pointer p, const T& value) {
        new (p) T(value); // Вызов placement new
    }

    void destroy(pointer p) {
        p->~T(); // Вызов деструктора объекта
    }
};

// Шаблонный класс контейнера
template <typename T, typename Allocator = std::allocator<T>>
class MyContainer {
public:
    using value_type = T;
    using allocator_type = Allocator;
    using size_type = std::size_t;
    using iterator = T*;
    using const_iterator = const T*;

private:
    Allocator alloc;
    T* data;
    size_type size_;

public:
    // Конструктор без аргументов
    MyContainer(const Allocator& alloc = Allocator()) : alloc(alloc), data(nullptr), size_(0) {}

    // Конструктор копирования
    MyContainer(const MyContainer& other) : alloc(other.alloc), size_(other.size_) {
        data = alloc.allocate(size_);
        for (size_type i = 0; i < size_; ++i) {
            alloc.construct(data + i, other.data[i]);
        }
    }

    // Оператор присваивания
    MyContainer& operator=(const MyContainer& other) {
        if (this != &other) {
            // Освобождаем старую память
            for (size_type i = 0; i < size_; ++i) {
                alloc.destroy(data + i);
            }
            alloc.deallocate(data, size_);

            // Выделяем новую память и копируем элементы
            alloc = other.alloc;
            size_ = other.size_;
            data = alloc.allocate(size_);
            for (size_type i = 0; i < size_; ++i) {
                alloc.construct(data + i, other.data[i]);
            }
        }
        return *this;
    }

    // Деструктор
    ~MyContainer() {
        for (size_type i = 0; i < size_; ++i) {
            alloc.destroy(data + i);
        }
        alloc.deallocate(data, size_);
    }

    // Добавление элемента в конец
    void push_back(const T& value) {
        // Выделение памяти для нового элемента
        T* newData = alloc.allocate(size_ + 1);
        // Копирование старых элементов
        for (size_type i = 0; i < size_; ++i) {
            alloc.construct(newData + i, data[i]);
        }
        // Конструирование нового элемента
        alloc.construct(newData + size_, value);
        // Освобождение старой памяти
        alloc.deallocate(data, size_);
        // Обновление данных
        data = newData;
        size_++;
    }

    // Метод для вывода элементов
    void display() const {
        for (size_type i = 0; i < size_; ++i) {
            std::cout << data[i] << " ";
        }
        std::cout << std::endl;
    }

    // Доступ к элементам по индексу
    T& operator[](size_type index) {
        return data[index];
    }

    // const-доступ к элементам по индексу
    const T& operator[](size_type index) const {
        return data[index];
    }

    // Получение размера контейнера
    size_type size() const {
        return size_;
    }
};

// Факториал
int factorial(int n) {
    if (n == 0) {
        return 1;
    }
    return n * factorial(n - 1);
}

int main() {
    // Используем стандартный аллокатор
    std::map<int, int> standardMap;
    // Используем собственный аллокатор
    using CustomMap = std::map<int, int, std::less<int>, CustomAllocator<std::pair<const int, int>>>;
    CustomMap customMap;

    // Заполняем стандартный map факториалами
    for (int i = 0; i < 10; ++i) {
        standardMap[i] = factorial(i);
    }

    std::cout << "Standard map values:" << std::endl;
    for (const auto& pair : standardMap) {
        std::cout << pair.first << " " << pair.second << std::endl;
    }

    // Создаем стандартный контейнер 
    MyContainer<int> myContainer;

    // Заполняем стандартный контейнер
    for (int i = 0; i < 10; ++i) { 
        myContainer.push_back(i); 
    }

    // Создаем собственный контейнер с собственным аллокатором
    MyContainer<int, CustomAllocator<int>> myCustomContainer;

    // Заполняем собственный контейнер
    for (int i = 0; i < 10; ++i) { 
        myCustomContainer.push_back(i); 
    }

    // Выводим стандартный контейнер
    std::cout << "My container values:" << std::endl;
    myContainer.display(); 

    // Выводим собственный контейнер с 20 элементами
    std::cout << "My custom container with 10 elements:" << std::endl;
    myCustomContainer.display(); 

    // Создаем собственный словарь с собственным аллокатором
    std::map<int, int, std::less<int>, CustomAllocator<std::pair<const int, int>>> myCustomDictionary;

    // Заполняем словарь факториалами
    for (int i = 0; i < 14; ++i) {
        myCustomDictionary[i] = factorial(i);
    }

    // Выводим словарь
    std::cout << "My custom dictionary with 14 elements (factorials):" << std::endl;
    for (const auto& pair : myCustomDictionary) {
        std::cout << pair.first << " " << pair.second << std::endl;
    }

    return 0;
}
