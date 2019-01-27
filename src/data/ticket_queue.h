#ifndef TASK_QUEUE_H
#define TASK_QUEUE_H

#include "task_master2.h"

namespace thrd
{
namespace details
{

enum wanted_t
{
    // перейти в состояние job_pending
    //
    // вызывается `job::notify_restart`
    want_reset,


    // остановиться и начать обработку заново, кроме `stopped_*`
    // если задание работает job_active, job_finished, вызывается want_reset
    // затем отрабатывается сам тикет
    //
    // вызывается `job::notify_process_start`
    want_restart,

    // перейти в состояние job_stopped
    // полностью аналогичен want_reset, кроме конечного состояния
    //
    // вызывается `job::notify_restart`
    want_stop,

    // перейти в состояние job_stopped_noparent,
    // в остальном полностью аналогичен предыдущему
    //
    // вызывается `job::notify_restart`
    want_stop_noparent,

    // выйти из состояния job_stopped
    // снимает состояние job_stopped -> job_pending
    // затем пренаправляется на want_restart
    want_resume,

    want_set_parent, // остановиться и заменить родителя
    want_remove,     // остановить и удалить всяческие упоминания

    want_action      // просто выполнять
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
        // стабильная сортировка по приоритету
        std::stable_sort(STL_II(tickets_), piority_first);

        for(unsigned index=0; index < tickets_.size(); ++index)
        {
            if (job_set.count( tickets_[index].job1 ) == 0)
            {
                // не обработана
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
