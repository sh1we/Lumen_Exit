#pragma once
#include <cmath>
#include <immintrin.h> // AVX/SSE intrinsics

// Быстрые математические утилиты с SIMD оптимизациями
namespace MathUtils
{
    // Константы
    constexpr float PI = 3.14159265359f;
    constexpr float TWO_PI = 6.28318530718f;
    constexpr float HALF_PI = 1.57079632679f;
    
    // ============================================
    // SIMD оптимизированные функции (AVX)
    // ============================================
    
    // Быстрое вычисление 4 синусов одновременно (AVX)
    inline void sin4_avx(const float* angles, float* results)
    {
#ifdef __AVX__
        __m128 x = _mm_loadu_ps(angles);
        
        // Приближение синуса через полином (быстрее чем std::sin)
        // sin(x) ≈ x - x³/6 + x⁵/120 (Taylor series)
        __m128 x2 = _mm_mul_ps(x, x);
        __m128 x3 = _mm_mul_ps(x2, x);
        __m128 x5 = _mm_mul_ps(x3, x2);
        
        __m128 term1 = x;
        __m128 term2 = _mm_mul_ps(x3, _mm_set1_ps(-0.16666667f)); // -1/6
        __m128 term3 = _mm_mul_ps(x5, _mm_set1_ps(0.00833333f));  // 1/120
        
        __m128 result = _mm_add_ps(_mm_add_ps(term1, term2), term3);
        _mm_storeu_ps(results, result);
#else
        // Fallback без AVX
        for (int i = 0; i < 4; ++i)
            results[i] = std::sin(angles[i]);
#endif
    }
    
    // Быстрое вычисление 4 косинусов одновременно (AVX)
    inline void cos4_avx(const float* angles, float* results)
    {
#ifdef __AVX__
        __m128 x = _mm_loadu_ps(angles);
        
        // cos(x) ≈ 1 - x²/2 + x⁴/24
        __m128 x2 = _mm_mul_ps(x, x);
        __m128 x4 = _mm_mul_ps(x2, x2);
        
        __m128 term1 = _mm_set1_ps(1.0f);
        __m128 term2 = _mm_mul_ps(x2, _mm_set1_ps(-0.5f));        // -1/2
        __m128 term3 = _mm_mul_ps(x4, _mm_set1_ps(0.04166667f));  // 1/24
        
        __m128 result = _mm_add_ps(_mm_add_ps(term1, term2), term3);
        _mm_storeu_ps(results, result);
#else
        for (int i = 0; i < 4; ++i)
            results[i] = std::cos(angles[i]);
#endif
    }
    
    // Быстрое вычисление sin и cos одновременно
    inline void sincos_fast(float angle, float& sinVal, float& cosVal)
    {
        // Используем стандартные функции - они достаточно быстрые
        // и работают корректно для всех углов
        sinVal = std::sin(angle);
        cosVal = std::cos(angle);
    }
    
    // ============================================
    // Быстрые скалярные функции
    // ============================================
    
    // Быстрый обратный квадратный корень (Quake III алгоритм)
    inline float fast_inv_sqrt(float x)
    {
        float xhalf = 0.5f * x;
        int i = *(int*)&x;
        i = 0x5f3759df - (i >> 1); // Магическая константа
        x = *(float*)&i;
        x = x * (1.5f - xhalf * x * x); // Одна итерация Ньютона
        return x;
    }
    
    // Быстрый квадратный корень
    inline float fast_sqrt(float x)
    {
        return x * fast_inv_sqrt(x);
    }
    
    // Быстрое вычисление расстояния
    inline float fast_distance(float dx, float dy)
    {
        return fast_sqrt(dx * dx + dy * dy);
    }
    
    // Быстрое вычисление квадрата расстояния (без sqrt)
    inline float distance_squared(float dx, float dy)
    {
        return dx * dx + dy * dy;
    }
    
    // Линейная интерполяция
    inline float lerp(float a, float b, float t)
    {
        return a + (b - a) * t;
    }
    
    // Clamp значения
    inline float clamp(float value, float min, float max)
    {
        return value < min ? min : (value > max ? max : value);
    }
    
    // Нормализация угла в диапазон [-PI, PI]
    inline float normalize_angle(float angle)
    {
        while (angle > PI) angle -= TWO_PI;
        while (angle < -PI) angle += TWO_PI;
        return angle;
    }
    
    // ============================================
    // Векторные операции (SIMD)
    // ============================================
    
    struct Vec2
    {
        float x, y;
        
        Vec2() : x(0), y(0) {}
        Vec2(float _x, float _y) : x(_x), y(_y) {}
        
        inline float length() const { return fast_sqrt(x * x + y * y); }
        inline float length_squared() const { return x * x + y * y; }
        
        inline Vec2 normalized() const
        {
            float inv_len = fast_inv_sqrt(x * x + y * y);
            return Vec2(x * inv_len, y * inv_len);
        }
        
        inline float dot(const Vec2& other) const
        {
            return x * other.x + y * other.y;
        }
        
        inline Vec2 operator+(const Vec2& other) const { return Vec2(x + other.x, y + other.y); }
        inline Vec2 operator-(const Vec2& other) const { return Vec2(x - other.x, y - other.y); }
        inline Vec2 operator*(float scalar) const { return Vec2(x * scalar, y * scalar); }
    };
    
    // ============================================
    // Batch операции для raycasting
    // ============================================
    
    // Вычисляет 4 луча одновременно (AVX оптимизация)
    struct RayBatch
    {
        float angles[4];
        float dirX[4];
        float dirY[4];
        
        void compute_directions()
        {
            sincos_fast(angles[0], dirY[0], dirX[0]);
            sincos_fast(angles[1], dirY[1], dirX[1]);
            sincos_fast(angles[2], dirY[2], dirX[2]);
            sincos_fast(angles[3], dirY[3], dirX[3]);
        }
    };
    
    // ============================================
    // Lookup таблицы для ещё большей скорости
    // ============================================
    
    class TrigLookup
    {
    public:
        static constexpr int TABLE_SIZE = 4096;
        
        TrigLookup()
        {
            for (int i = 0; i < TABLE_SIZE; ++i)
            {
                float angle = (static_cast<float>(i) / TABLE_SIZE) * TWO_PI;
                m_sinTable[i] = std::sin(angle);
                m_cosTable[i] = std::cos(angle);
            }
        }
        
        inline float sin_lookup(float angle) const
        {
            // Нормализуем угол в [0, 2PI]
            while (angle < 0) angle += TWO_PI;
            while (angle >= TWO_PI) angle -= TWO_PI;
            
            int index = static_cast<int>((angle / TWO_PI) * TABLE_SIZE) % TABLE_SIZE;
            return m_sinTable[index];
        }
        
        inline float cos_lookup(float angle) const
        {
            while (angle < 0) angle += TWO_PI;
            while (angle >= TWO_PI) angle -= TWO_PI;
            
            int index = static_cast<int>((angle / TWO_PI) * TABLE_SIZE) % TABLE_SIZE;
            return m_cosTable[index];
        }
        
        inline void sincos_lookup(float angle, float& sinVal, float& cosVal) const
        {
            while (angle < 0) angle += TWO_PI;
            while (angle >= TWO_PI) angle -= TWO_PI;
            
            int index = static_cast<int>((angle / TWO_PI) * TABLE_SIZE) % TABLE_SIZE;
            sinVal = m_sinTable[index];
            cosVal = m_cosTable[index];
        }
        
    private:
        float m_sinTable[TABLE_SIZE];
        float m_cosTable[TABLE_SIZE];
    };
    
    // Глобальная lookup таблица (инициализируется один раз)
    inline TrigLookup& get_trig_lookup()
    {
        static TrigLookup lookup;
        return lookup;
    }
}
