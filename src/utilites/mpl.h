//////////////////////////////////////////////////////////////////////////
//
// utilites library
//
// Written by Pavel Amialiushka
// No commercial use permited.
//
//////////////////////////////////////////////////////////////////////////

#pragma  once

namespace mpl
{


// ���� TrueType � FalseType ����� ���� ���������� ������������ �������,
// ������� ����� ����������� �������: sizeof(TrueType) != sizeof(FalseType).
struct TrueType
{
    char dummy_ [1];
};
struct FalseType
{
    char dummy_ [2];
};
template<bool> struct bool_ {};

// ������������� ����� PointerShim �����,
// ����� �������� ��������� ������ �����������
// IsPointer � ������ �������������� �������, � ������� ���������
// �������� �������������� � ���������.
struct PointerShim
{
    PointerShim(const volatile void *);
    PointerShim(volatile void *);
    PointerShim(const void *);
    PointerShim(void *);
};

// �.�. ������� ptr_discriminator �� ����� ����
// �� ����������, ���������� �� ���������.
TrueType  ptr_discriminator(PointerShim);
FalseType ptr_discriminator(...);

// IsPointer<T>::value == true, ���� T �������� ����������,
// IsPointer<T>::value == false � ��������� ������.
template<class T>
class IsPointer
{
private:
    static T t_;

public:
    enum { value = sizeof(ptr_discriminator(t_)) == sizeof(TrueType) };
    typedef bool_<value> ret;
};

// ��� ��� ������ ���� void ������ ���� �� �����,
// ������ IsPointer<void> ������ �������������� ��������.
template<>
class IsPointer<void>
{
public:
    enum { value = false };
    typedef bool_<value> ret;
};

//////////////////////////////////////////////////////////////////////////

TrueType  ptr_like_discriminator(const volatile void *);
FalseType ptr_like_discriminator(...);

// IsPointer<T>::value == true, ���� T �������� ����������,
// IsPointer<T>::value == false � ��������� ������.
template<class T>
class IsPtrLike
{
private:
    static T t_;

public:
    enum { value = sizeof(ptr_like_discriminator(t_)) == sizeof(TrueType) };
    typedef bool_<value> ret;
};

// ��� ��� ������ ���� void ������ ���� �� �����,
// ������ IsPointer<void> ������ �������������� ��������.
template<>
class IsPtrLike<void>
{
public:
    enum { value = false };
    typedef bool_<false> ret;
};


//////////////////////////////////////////////////////////////////////////

TrueType  const_discriminator(const volatile void *);
FalseType const_discriminator(volatile void *);

template<class T> struct IsConst
{
private:
    static T t_;

public:
    enum { value = sizeof(const_discriminator(&t_)) == sizeof(TrueType) };
};

template<>
struct IsConst<void>
{
public:
    enum { value = false };
};

TrueType  volatile_discriminator(const volatile void *);
FalseType volatile_discriminator(const void *);

template<class T>
struct IsVolatile
{
private:
    static T t_;

public:
    enum { value = sizeof(volatile_discriminator(&t_)) == sizeof(TrueType) };
};

template<>
struct IsVolatile<void>
{
public:
    enum { value = false };
};


//////////////////////////////////////////////////////////////////////////

template<class T>
class IsReference
{
private:
    typedef T const volatile cv_t_;

public:
    enum
    {
        value = !IsConst<cv_t_>::value || !IsVolatile<cv_t_>::value
    };
};

template<>
class IsReference<void>
{
public:
    enum { value = false };
};

}