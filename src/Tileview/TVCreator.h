#pragma once

#include "utilites/serl/Archive.h"

typedef void *(*make_object_t)();
typedef HWND  (*make_window_t)(void *, HWND);
typedef void  (*make_serial_t)(void *, serl::archiver &);

struct CTVCreator
{
    CTVCreator();
    CTVCreator(make_object_t o, make_window_t w, make_serial_t s);
    virtual ~CTVCreator();

    void create();
    HWND activate(HWND hwnd);
    void serialization(serl::archiver &ar);
protected:

    void         *object_;
    make_object_t make_object_;
    make_window_t make_window_;
    make_serial_t make_serial_;
};

//////////////////////////////////////////////////////////////////////////

template<class T>
struct CTVCreatorImpl : CTVCreator
{
    CTVCreatorImpl(T *pointer=0)
        : CTVCreator(
            &make_object,
            &make_window,
            &make_serial
        )
    {
        object_=pointer;
    }

    static void *make_object()
    {
        return new T;
    }

    static HWND make_window(void *object, HWND hparent)
    {
        T *self=reinterpret_cast<T *>(object);

        debug::Assert<fault>(self, HERE);
        return self->Create(hparent, CWindow::rcDefault);
    }

    static void make_serial(void *object, serl::archiver &ar)
    {
        T *self=reinterpret_cast<T *>(object);
        ar.serial(*self);
    }
};

//////////////////////////////////////////////////////////////////////////
