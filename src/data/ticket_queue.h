#ifndef TASK_QUEUE_H
#define TASK_QUEUE_H

#include "task_master2.h"

namespace thrd
{
namespace details
{

enum wanted_t
{
    // ������� � ��������� job_pending
    //
    // ���������� `job::notify_restart`
    want_reset,


    // ������������ � ������ ��������� ������, ����� `stopped_*`
    // ���� ������� �������� job_active, job_finished, ���������� want_reset
    // ����� �������������� ��� �����
    //
    // ���������� `job::notify_process_start`
    want_restart,

    // ������� � ��������� job_stopped
    // ��������� ���������� want_reset, ����� ��������� ���������
    //
    // ���������� `job::notify_restart`
    want_stop,

    // ������� � ��������� job_stopped_noparent,
    // � ��������� ��������� ���������� �����������
    //
    // ���������� `job::notify_restart`
    want_stop_noparent,

    // ����� �� ��������� job_stopped
    // ������� ��������� job_stopped -> job_pending
    // ����� ��������������� �� want_restart
    want_resume,

    want_set_parent, // ������������ � �������� ��������
    want_remove,     // ���������� � ������� ��������� ����������

    want_action      // ������ ���������
};

struct ticket
{
    wanted_t want;
    job*     job1;
    job*     job2;
    int      priority;
};

enum action_t
{
    its_time_to_action,
    skip_this_action,
    cannot_proceed_right_now
};

class ticket_queue
{
    std::deque<ticket> tickets_;
public:

    bool empty() const
    {
        return tickets_.empty();
    }

    bool contains(job* j1) const
    {
        for(unsigned index=0; index < tickets_.size(); ++index)
        {
            if (tickets_[index].job1 == j1)
            {
                return true;
            }
        }
        return false;
    }

    void remove_all(job* j1)
    {
        for(unsigned index = 0; index < tickets_.size(); ++index)
        {
            if (tickets_[index].job1 == j1)
            {
                tickets_.erase( tickets_.begin() + index);
                --index;
            }
        }
    }

    friend bool piority_first(ticket tk1, ticket tk2)
    {
        return tk1.priority > tk2.priority;
    }

    bool take_ticket_not_from_set_with_priority(ticket& tk,
                                                std::set<job*>& job_set,
                                                int priority)
    {
        // ���������� ���������� �� ����������
        std::stable_sort(STL_II(tickets_), piority_first);

        for(unsigned index=0; index < tickets_.size(); ++index)
        {
            if (job_set.count( tickets_[index].job1 ) == 0)
            {
                // �� ����������
                tk = tickets_[index];

                if (priority>=0 && tk.priority!=priority)
                    return false;

                tickets_.erase(tickets_.begin() + index);

                job_set.insert( tk.job1 );
                return true;
            }
        }
        return false;
    }

    void push_back(ticket const& tk)
    {
        tickets_.push_back(tk);
    }

    void push_front(ticket const& tk)
    {
        tickets_.push_front(tk);
    }
};

}
}

#endif // TASK_QUEUE_H
