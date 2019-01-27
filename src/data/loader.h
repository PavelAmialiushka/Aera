#pragma once

#include "data/node.h"
#include "data/datafactory.h"


namespace nodes
{

MAKE_SHARED(loader_result);

class loader_result
        : public nodes::result
{
public:
    bool             sorted;
    data::collection collection_;
};

/**
 * @brief ��������� �������� ���� ����� ������ �� ��������� �����.
 *
 *
 */

class loader
        : public node
{
public:
    loader();
    ~loader();

    /**
     * @brief �������� �������� ���������� ������
     */
    void open(const std::string &filename);


    virtual void setup(process::hostsetup&);
    virtual process::processor* create_processor();

    void restart();

private:    
    struct config : process::config
    {
        std::string      filename_;
    } config_;

    struct processor;
    friend struct processor;

};

}
