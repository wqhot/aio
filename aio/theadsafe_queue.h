// Copyright (C) 2011 Paul Ilardi (http://github.com/CodePi)
// 
// Permission is hereby granted, free of charge, to any person obtaining a 
// copy of this software and associated documentation files (the "Software"),
// to deal in the Software without restriction, including without limitation 
// the rights to use, copy, modify, merge, publish, distribute, sublicense, 
// and/or sell copies of the Software, and to permit persons to whom the 
// Software is furnished to do so, unconditionally.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR 
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, 
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE 
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER 
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING 
// FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER 
// DEALINGS IN THE SOFTWARE.

#pragma once

#include <queue>
#include <stack>
#include <mutex>
#include <condition_variable>

namespace codepi {

    template <class T, class Container = std::queue<T>>
    class ThreadSafeQueue {
    public:

        ThreadSafeQueue() = default;

        // enqueue - supports move, copies only if needed. e.g. q.enqueue(move(obj));
        void enqueue(T t) {
            std::lock_guard<std::mutex> lock(m);
            while (size() > max_size)
            {
                pop();
            }
            q.push(std::move(t));
            c.notify_one();
        }

        // simple dequeue
        T dequeue() {
            std::unique_lock<std::mutex> lock(m);
            while (empty()) c.wait(lock);
            return pop();
        }

        // dequeue with timeout in seconds
        bool dequeue(double timeout_sec, T& rVal) {
            std::unique_lock<std::mutex> lock(m);
            bool isTimeout = false;

            // wait for timeout or value available
            auto maxTime = std::chrono::milliseconds(int(timeout_sec * 1000));
            if (c.wait_for(lock, maxTime, [&]() {return !this->empty(); })) {
                rVal = pop();
                return true;
            }
            else {
                return false;
            }
        }

        size_t size() const { return q.size(); }
        bool  empty() const { return q.empty(); }
        void  clear() {
            std::lock_guard<std::mutex> lock(m);
            // stack and queue have no clear method!! Maybe switch to deque
            while (!empty()) pop();
        }

    private:
        const long max_size = 1024 * 1024;
        Container q;
        mutable std::mutex m;
        std::condition_variable c;

        T& get_next(std::stack<T>& s) { return s.top(); }
        T& get_next(std::queue<T>& qu) { return qu.front(); }

        T pop() {
            T val = std::move(get_next(q));
            q.pop();
            return val;
        }

    };

} // end namespace codepi