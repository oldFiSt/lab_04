#include <iostream> 
#include <map> 
#include <memory> 
#include <vector> 
#include <utility> 
#include <cstring> 


template <typename T>
class CustomAllocator {
public:
    using value_type = T; // Определяем тип значения

    CustomAllocator(size_t blockSize = 10) 
        : blockSize(blockSize), currentIndex(0), memory(nullptr) {
        allocateBlock(); 
    }

    // Добавляем копирующий конструктор
    CustomAllocator(const CustomAllocator& other)
        : blockSize(other.blockSize), currentIndex(other.currentIndex), memory(nullptr) {
        allocateBlock(); // Выделяем новый блок памяти
        std::memcpy(memory.get(), other.memory.get(), currentIndex * sizeof(T)); // Копируем данные
    }

    ~CustomAllocator() {
        deallocateBlock(); // Освобождаем выделенный блок памяти
    }

    T* allocate(std::size_t n) {
        if (currentIndex + n > blockSize) {
            expand(); // Если нет, расширяем блок памяти
        }
        T* result = reinterpret_cast<T*>(memory.get() + currentIndex * sizeof(T));
        currentIndex += n; // Увеличиваем текущий индекс
        return result; // Возвращаем указатель на выделенную память
    }

    void deallocate(T* p, std::size_t n) {
    if (p != nullptr) {
        ::operator delete(p); // Освобождаем память
    }
}


private:
    size_t blockSize; // Размер блока памяти
    size_t currentIndex; // Текущий индекс в блоке памяти
    std::unique_ptr<char[]> memory; // Умный указатель на выделенную память

    void allocateBlock() {
        memory.reset(new char[blockSize * sizeof(T)]); // Выделяем блок памяти
        currentIndex = 0; // Сбрасываем текущий индекс
    }

    void deallocateBlock() {
        memory.reset(); // Умный указатель автоматически освободит память
    }

    void expand() {
        size_t newSize = blockSize * 2; // Увеличиваем размер блока вдвое
        std::unique_ptr<char[]> newMemory(new char[newSize * sizeof(T)]);
        std::memcpy(newMemory.get(), memory.get(), currentIndex * sizeof(T)); // Копируем старые данные в новый блок
        memory.swap(newMemory); // Меняем старый блок на новый
        blockSize = newSize; // Обновляем размер блока
    }
};

// Определяем пользовательский контейнер, параметризуемый типом и аллокатором
template <typename T, typename Allocator = std::allocator<T>>
class MyContainer {
public:
    using allocator_type = Allocator; // Определяем тип аллокатора

    MyContainer(Allocator alloc = Allocator()) : allocator(alloc) {} // Конструктор с параметром аллокатора

    void push_back(const T& value) { // Метод для добавления элемента в контейнер
        T* newElement = allocator.allocate(1); // Выделяем память под новый элемент с помощью аллокатора
        new (newElement) T(value); // Размещаем новый элемент в выделенной памяти (placement new)
        elements.push_back(newElement); // Добавляем указатель на новый элемент в вектор элементов
    }

    void pop_back() { // Метод для удаления последнего элемента из контейнера
        if (!elements.empty()) {
            allocator.deallocate(elements.back(), 1); // Освобождаем память последнего элемента
            elements.pop_back(); // Удаляем указатель из вектора элементов
        }
    }

    void display() const { // Метод для отображения всех элементов контейнера
        for (const auto& elem : elements) { 
            std::cout << *elem << " "; 
        }
        std::cout << std::endl; 
    }

private:
    std::vector<T*> elements; // Вектор указателей на элементы контейнера
    Allocator allocator; // Аллокатор, используемый для управления памятью
};

// Функция для вычисления факториала числа рекурсивно
int factorial(int n) {
    return (n <= 1) ? 1 : n * factorial(n - 1); 
}

int main() {
    std::map<int, int> standardMap;

    for (int i = 0; i < 10; ++i) {
        standardMap[i] = factorial(i);
    }

    using CustomMap = std::map<int, int, std::less<int>, CustomAllocator<std::pair<const int, int>>>;
    CustomMap customMap;

    for (int i = 0; i < 10; ++i) {
        customMap[i] = factorial(i);
    }

    std::cout << "Standard map values:" << std::endl;
    for (const auto& pair : standardMap) { 
        std::cout << pair.first << " " << pair.second << std::endl; 
    }

    MyContainer<int> myContainer;

    for (int i = 0; i < 10; ++i) { 
        myContainer.push_back(i); 
    }

  	MyContainer<int, CustomAllocator<int>> myCustomContainer;

  	for (int i = 0; i < 10; ++i) { 
      	myCustomContainer.push_back(i); 
  	}

  	std::cout << "My container values:" << std::endl;
  	myContainer.display(); 

  	return 0;
}
