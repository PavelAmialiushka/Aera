#pragma once

//////////////////////////////////////////////////////////////////////////

class object_id
{
    object_id(int)
    {
        ::CoCreateGuid(& guid_);
    }

public:

    object_id()
    {
        ::ZeroMemory(&guid_, sizeof(guid_));
    }

    object_id(std::string string)
    {
        std::vector<wchar_t> buffer(string.size()+1);
        std::copy(STL_II(string), buffer.begin());
        buffer.back()=0;

        CLSIDFromString(&buffer[0], &guid_);
    }

    object_id(const object_id &other)
        : guid_(other.guid_)
    {
    }

    object_id &operator=(const object_id &other)
    {
        guid_=other.guid_;
        return *this;
    }

    bool empty() const
    {
        return *this == object_id();
    }

    bool operator==(const object_id &other) const
    {
        return guid_==other.guid_;
    }

    bool operator!=(const object_id &other) const
    {
        return guid_!=other.guid_;
    }

    bool operator<(object_id const &other) const
    {
        return memcmp(&guid_, &other.guid_, sizeof(GUID)) < 0;
    }

    static object_id create()
    {
        return object_id(int());
    }

    std::string string() const
    {
        std::vector<wchar_t> buffer(40);

        StringFromGUID2(guid_, &buffer[0], buffer.size());

        std::vector<char> foobar(40); std::copy(STL_II(buffer), foobar.begin());
        return std::string( &foobar[0] );
    }

    template<typename T> void serialization(T &ar)
    {
        std::string value;

        if (ar.is_saving()) value=string();

        ar.serial("value", value);

        if (ar.is_loading()) *this=object_id(value);
    }

private:

    GUID guid_;

};
