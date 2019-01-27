#pragma once

enum { MAX_CHANNEL_NUMBER = 64 };

// группа каналов
struct channel_object
{
    std::string      name;
    std::vector<int> channels;
    boost::array<int, MAX_CHANNEL_NUMBER> isactive_;

    bool isactive(int) const;
    void setactive(int, bool);

    bool hasinactive() const;
    bool hasactive() const;
    void setactive(bool);
};

// группа объектов
struct channel_sheme
{
    channel_sheme();

    std::vector<channel_object> objects;

    bool isactive(int ch) const;
    void setactive(int ch, bool);

    static channel_sheme default_sheme();
};
