#include "stdafx.h"

#include "serl/element.h"

namespace serl
{

element::element(std::string n)
    : name_(n)
{
}

std::string element::name() const
{
    return name_;
}

std::string element::value() const
{
    if (is_named_value())
        return children_.front()->name();

    return std::string();
}

bool element::is_folder() const
{
    return !is_value() && !is_named_value();
}

bool element::is_named_value() const
{
    return children_.size()
            && children_.front()->is_value();
}

bool element::is_value() const
{
    return children_.empty();
}

bool element::lookup(std::string name, pelement &result)
{
    const std::string sep = "\\";

    if (name.empty())
    {
        result = shared_from_this();
        return true;
    }

    // поиск пути
    if (name.find(sep)!=std::string::npos)
    {
        std::string path = strlib::split(name, sep);
        pelement child;
        if (lookup(path, child))
        {
            return child->lookup(name, result);
        }
        return false;
    }

    // путь найден, перебираем элементы
    for (unsigned index=0; index<children_.size(); ++index)
    {
        pelement element=children_[index];
        if (element->name()==name)
        {
            result = element;
            return true;
        }
    }
    return false;

}

pelement element::lookup(std::string name)
{
    // ищем элемент с заданным номером
    pelement result;
    if (lookup(name, result))
        return result;

    // вставляем новый элемент
    result.reset( new element(name) );
    children_.push_back(result);
    return result;
}

void element::insert(std::string name, std::string value)
{
    pelement child = lookup(name);
    child->set_value(value);
}

void element::set_value(std::string value)
{
    pelement child(new element(value));
    if (children_.size())
        children_[0] = child;
    else
        children_.push_back( child );
}

pelement element::at(unsigned index) const
{
    return children_[index];
}

unsigned element::size() const
{
    return children_.size();
}

pelement element::take(std::string path)
{
    const std::string sep = "\\";

    std::string base = strlib::split(path, sep);
    if (!path.empty())
    {
        pelement child;
        if (lookup(base, child))
            return child->take(path);

        // путь не найден
        return pelement();
    }

    // предпоследний элемент
    pelement child;
    if (lookup(base, child))
    {
        // удаляем элемент
        children_.erase( std::find(STL_II(children_), child) );

        return child;
    }

    return pelement();
}

void element::insert(std::string where, pelement node)
{
    const std::string sep = "\\";

    pelement child = lookup(where);
    child->children_.push_back(node);
}

}
