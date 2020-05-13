#ifndef RTEYUWIERU
#define RTEYUWIERU

#include <cstdlib>
#include <stdexcept>
#include <new>

struct IllegalOperationOnCurrentState : std::runtime_error {
    using std::runtime_error::runtime_error;
};
#endif

class Vector {
    T* arr;
    size_t inUse;
    size_t allocated;

    constexpr static const size_t DEFAULT_CAPACITY = 2;

    // destroy: frees the arr
    // for object types, it loops through the array and calls the destructor for each element
    // before calling free
    inline void destroy() {
        if constexpr (std::is_object_v<T>)
            std::destroy(arr, arr+inUse);
        free(arr);
    }

    //initializes the vector
    inline void init(size_t capacity) {
        // use malloc to allocate the array, so that the default constructor is not called.
        // T may not be default constructible.
        // Also, copy-constructing / move-constructing the objects is better than
        // default-constructing them and then copy-assigning / move-assigning them on calls to push and emplace.
        arr = (T*) malloc(sizeof(T) * (allocated = capacity));
        inUse = 0;
    }

    void move(Vector&& other) {
        arr = other.arr;
        inUse = other.inUse;
        allocated = other.allocated;
        other.init(DEFAULT_CAPACITY);
    }

    void copy(const Vector& other) {
        arr = (T*) malloc(sizeof(T) * (allocated = other.allocated));
        inUse = other.inUse;

        for (size_t i = 0; i < other.inUse; ++i)
            new (arr + i) T(other.arr[i]); //calling copy constructor
    }

public:
    using value_type = T;
    using size_type = size_t;

    //constructor: takes an optional capacity argument
    explicit Vector(size_t capacity = DEFAULT_CAPACITY) { init(capacity); }

    //copy constructor
    Vector(const Vector& other) { copy(other); }

    //move constructor
    Vector(Vector&& other) noexcept { move(std::forward<Vector>(other)); }

    //copy assignment operator
    Vector& operator=(const Vector& other) { destroy(); copy(other); return *this; }

    //move assignment operator
    Vector& operator=(Vector&& other)  noexcept { destroy(); move(std::forward<Vector>(other)); return *this; }

    //destructor
    ~Vector() { destroy(); }

    //resets the state of the Vector to it's default state
    void reset() { destroy(); init(DEFAULT_CAPACITY); }

    void push_back(const T& val) {
        if (inUse == allocated)
            arr = (T*) realloc(arr, sizeof(T) * (allocated += allocated));

        new (arr + inUse++) T(val); //calling new placement operator
    }

    template<class... Args>
    void emplace_back(Args&&... args) {
        if (inUse == allocated)
            arr = (T*) realloc(arr, sizeof(T) * (allocated += allocated));

        new (arr + inUse++) T(std::forward<Args>(args)...);
    }

    inline void push_back(T&& val) { emplace_back(val); }

    T& back() {
        if (inUse)
            return arr[inUse-1];
        throw IllegalOperationOnCurrentState("Container is empty");
    }

    const T& back() const {
        if (inUse)
            return arr[inUse-1];
        throw IllegalOperationOnCurrentState("Container is empty");
    }

    T& front() {
        if (inUse)
            return arr[0];
        throw IllegalOperationOnCurrentState("Container is empty");
    }

    const T& front() const {
        if (inUse)
            return arr[0];
        throw IllegalOperationOnCurrentState("Container is empty");
    }

    void pop_back() { std::destroy_at(arr + --inUse); }

    // unsafe: does not check for out of range values. Use only when sure that index is valid.
    inline T& operator[](size_t idx) { return arr[idx]; }

    inline const T& operator[](size_t idx) const { return arr[idx]; }

    T& at(size_t idx) {
        if (idx < inUse)
            return arr[idx];
        throw std::out_of_range("Index is out of bounds.");
    }

    [[nodiscard]] const T& at(size_t idx) const {
        if (idx < inUse)
            return arr[idx];
        throw std::out_of_range("Index is out of bounds.");
    }

    [[nodiscard]] constexpr bool empty() const { return inUse == 0; }

    [[nodiscard]] constexpr size_t size() const { return inUse; }

    [[nodiscard]] constexpr size_t capacity() const { return allocated; }

    template<class Ch, class Tr>
    friend std::basic_ostream<Ch, Tr>& operator<<(std::basic_ostream<Ch, Tr>& os, const Vector& v) {
        os << '[';
        if (v.inUse) {
            os << v.arr[0];
            for (size_t i = 1; i < v.inUse; ++i)
                os << ", " << v.arr[i];
        }
        return os << ']';
    }
};
