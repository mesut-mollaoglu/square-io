#ifndef MATH_H
#define MATH_H

#include "includes.h"

template <class T> struct Vector2D
{
    static_assert(std::is_arithmetic_v<T>);
    T x, y;
    inline Vector2D() = default;
    inline Vector2D(const T _v) : x(_v), y(_v) {}
    inline Vector2D(const Vector2D<T>& v2d) : x(v2d.x), y(v2d.y) {}
    inline Vector2D(const T _x, const T _y) : x(_x), y(_y) {}
    inline Vector2D& operator=(const Vector2D& v2d) = default;
    inline friend void operator*=(Vector2D& v2d, const T _v) 
    {
        v2d.x *= _v;
        v2d.y *= _v;
    }
    inline friend void operator/=(Vector2D& v2d, const T _v) 
    {
        v2d.x /= _v;
        v2d.y /= _v;
    }
    inline friend void operator+=(Vector2D& v2d, const Vector2D v2d_2)
    {
        v2d.x += v2d_2.x;
        v2d.y += v2d_2.y;
    }
    inline friend void operator-=(Vector2D& v2d, const Vector2D v2d_2)
    {
        v2d.x -= v2d_2.x;
        v2d.y -= v2d_2.y;
    }
    inline friend Vector2D operator*(const Vector2D& v2d, const T _v)
    {
        return Vector2D<T>(
            v2d.x * _v,
            v2d.y * _v
        );
    }
    inline friend Vector2D operator/(const Vector2D& v2d, const T _v)
    {
        return Vector2D<T>(
            v2d.x / _v,
            v2d.y / _v
        );
    }
    inline friend Vector2D operator+(const Vector2D& v2d, const Vector2D v2d_2)
    {
        return Vector2D<T>(
            v2d.x + v2d_2.x,
            v2d.y + v2d_2.y
        );
    }
    inline friend Vector2D operator-(const Vector2D& v2d, const Vector2D v2d_2)
    {
        return Vector2D<T>(
            v2d.x - v2d_2.x,
            v2d.y - v2d_2.y
        );
    }
    inline friend Vector2D operator-(const Vector2D& v2d)
    {
        return Vector2D<T>(-v2d.x, -v2d.y);
    }
    template <class F> inline operator Vector2D<F>()
    {
        x = static_cast<F>(x);
        y = static_cast<F>(y);
    }
    inline std::array<T, 2> data()
    {
        return {x, y};
    }
};

typedef Vector2D<float> v2f;
typedef Vector2D<double> v2d;
typedef Vector2D<int> v2i;

template <class T, std::size_t size> inline T dot(std::array<T, size> a, std::array<T, size> b){
    T result = T(0);
    for(int i = 0; i < size; i++)
    {
        result += a[i] * b[i];
    }
    return result;
}

template <class T, std::size_t size> struct matrix
{
    static_assert(std::is_arithmetic_v<T>);
    T data[size][size];
    inline matrix() {memset(data, 0, sizeof(T) * size * size);}
    inline matrix(const matrix& mat){memcpy(data, mat.data, sizeof(T) * size * size);}
    inline matrix(const std::array<T, size * size> arr){memcpy(data, arr.data(), sizeof(T) * size * size);}
    matrix& operator=(const matrix& mat) = default;
    inline matrix(const T arr[size * size])
    {
        for(int x = 0; x < size; x++)
            for(int y = 0; y < size; y++)
                data[x][y] = arr[y * size + x];
    }
    inline matrix(T value) 
    {
        for(int x = 0; x < size; x++)
            for(int y = 0; y < size; y++)
                data[x][y] = (x == y) ? value : T(0);
    }
    inline std::array<T, size> row(int i) 
    {
        std::array<T, size> res; 
        for(int j = 0; j < size; j++) 
            res[j] = data[j][i];
        return res;
    }
    inline std::array<T, size> col(int i) 
    {
        std::array<T, size> res; 
        for(int j = 0; j < size; j++) 
            res[j] = data[i][j]; 
        return res;
    }
    inline void set_row(int r, std::array<T, size> row_data) 
    {
        for(int i = 0; i < size; i++) 
            data[i][r] = row_data[i];
    }
    inline void set_col(int c, std::array<T, size> col_data) 
    {
        for(int i = 0; i < size; i++) 
            data[c][i] = col_data[i];
    }
    friend bool operator==(const matrix& mat1, const matrix& mat2)
    {
        const int size1 = sizeof(mat1.data);
        const int size2 = sizeof(mat2.data);
        return (size1 == size2 && memcmp(mat1.data, mat2.data, size1) == 0);
    }
    friend bool operator!=(const matrix& mat1, const matrix& mat2)
    {
        return !(mat1 == mat2);
    }
    inline matrix<T, size> transpose()
    {
        matrix<T, size> res;
        for(int i = 0; i < size; i++)
            res.set_row(i, col(i));
        return res;
    }
    inline matrix inverse()
    {
        matrix<T, size> temp = *this;
        matrix<T, size> inv = matrix<T, size>(1.0);
        std::array<T, size> buffer;
        std::array<T, size> sub_row;
        for(int i = 0; i < size; i++)
        {
            buffer = temp.row(i);
            for(int j = 0; j < size; j++)
            {
                buffer[j] /= buffer[i];
                inv.data[i][j] /= buffer[i];
            }
            temp.set_row(i, buffer);
            for(int j = 0; j < size; j++)
            {
                if(i != j)
                {
                    sub_row = temp.row(j);
                    for(int k = 0; k < size; k++)
                    {
                        sub_row[k] -= buffer[k] * temp.data[i][j];
                        inv.data[j][k] -= temp.data[i][j] * inv.data[i][k];
                    }
                    temp.set_row(j, sub_row);
                }
            }
        }
        return inv.transpose();
    }
    inline matrix pow(int i)
    {
        if(i == 0)
        {
            return matrix(1.0);
        }
        else if(i == 1)
        {
            return *this;
        }
        else 
        {
            matrix mat = *this;
            for(int j = 1; j < i; j++)
                mat = mat * (*this);
            return mat;
        }
    }
    inline T determinant()
    {
        T determinant = 1;
        matrix<T, size> temp = *this;
        std::array<T, size> buffer;
        std::array<T, size> sub_row;
        for(int i = 0; i < size; i++)
        {
            buffer = temp.row(i);
            for(int j = i; j < size; j++)
            {
                buffer[j] /= buffer[i];
            }
            for(int j = i+1; j < size; j++)
            {
                sub_row = temp.row(j);
                for(int k = 0; k < size; k++)
                {
                    sub_row[k] -= buffer[k] * temp.data[i][j];
                }
                temp.set_row(j, sub_row);
            }
        }
        for(int i = 0; i < size; i++)
        {
            determinant *= temp.data[i][i];
        }
        return determinant;
    }
    friend matrix operator*(matrix mat1, matrix mat2)
    {
        matrix res;
        for(int i = 0; i < size; i++)
            for(int j = 0; j < size; j++)
                res.data[i][j] = dot(mat1.row(j), mat2.col(i));
        return res;
    }
    ~matrix() {}
};

typedef matrix<float, 2> matrix2x2f;
typedef matrix<float, 3> matrix3x3f;
typedef matrix<double, 3> matrix3x3d;

inline float deg2rad(float angle)
{
    return (angle / 180.f) * pi;
}

inline float rad2deg(float angle)
{
    return (angle / pi) * 180.f;
}

inline v2f rotate(float angle, v2f v)
{
    return v2f(
        cos(angle) * v.x - sin(angle) * v.y,
        sin(angle) * v.x + cos(angle) * v.y
    );
}

template <class T> inline T rand(T min, T max)
{
    return ((float)rand() / (float)RAND_MAX) * (max - min) + min;
}

struct Transform{
    matrix3x3f transform;
    matrix3x3f inverted;
    bool invertMatrix;
    Transform();
    void Rotate(float ang);
    void Translate(float dx, float dy);
    void Scale(float sx, float sy);
    void Forward(float x, float y, float& ox, float& oy);
    void Backward(float x, float y, float& ox, float& oy);
    void Reset();
    void Invert();
    ~Transform() {}
};

#endif

#ifdef MATH_H
#undef MATH_H

Transform::Transform()
{
    this->Reset();
}

void Transform::Rotate(float ang)
{
	matrix3x3f rotate = matrix3x3f(1.0);
    rotate.data[0][0] = cos(ang);
    rotate.data[1][0] = sin(ang);
    rotate.data[0][1] = -sin(ang);
    rotate.data[1][1] = cos(ang);
	transform = transform * rotate;
	invertMatrix = true;
}

void Transform::Translate(float dx, float dy)
{
    matrix3x3f translate = matrix3x3f(1.0);
    translate.data[2][0] = dx;
    translate.data[2][1] = dy;
    transform = transform * translate;
    invertMatrix = true;   
}

void Transform::Scale(float sx, float sy)
{
   	matrix3x3f scale = matrix3x3f(1.0);
   	scale.data[0][0] = sx;
    scale.data[1][1] = sy;
   	transform = transform * scale;
   	invertMatrix = true;
}

void Transform::Reset()
{
    transform = matrix3x3f(1.0);
    inverted = matrix3x3f(1.0);
    invertMatrix = false;
}

void Transform::Forward(float x, float y, float& ox, float& oy)
{
    float oz;
	ox = transform.data[0][0] * x + transform.data[1][0] * y + transform.data[2][0];
    oy = transform.data[0][1] * x + transform.data[1][1] * y + transform.data[2][1];
    oz = transform.data[0][2] * x + transform.data[1][2] * y + transform.data[2][2];
	ox /= (oz == 0 ? 1 : oz);
	oy /= (oz == 0 ? 1 : oz);
}

void Transform::Backward(float x, float y, float& ox, float& oy)
{
	float oz;
    ox = inverted.data[0][0] * x + inverted.data[1][0] * y + inverted.data[2][0];
    oy = inverted.data[0][1] * x + inverted.data[1][1] * y + inverted.data[2][1];
    oz = inverted.data[0][2] * x + inverted.data[1][2] * y + inverted.data[2][2];
	ox /= (oz == 0 ? 1 : oz);
	oy /= (oz == 0 ? 1 : oz);
}

void Transform::Invert()
{
	if(!invertMatrix) return;
	inverted = transform.inverse();
    invertMatrix = false;
}

#endif
