/*
 * Copyright (c) 2019, Tom Oleson <tom dot oleson at gmail dot com>
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 *   * Redistributions of source code must retain the above copyright notice,
 *     this list of conditions and the following disclaimer.
 *   * Redistributions in binary form must reproduce the above copyright
 *     notice, this list of conditions and the following disclaimer in the
 *     documentation and/or other materials provided with the distribution.
 *   * The names of its contributors may NOT be used to endorse or promote
 *     products derived from this software without specific prior written
 *     permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 */

#ifndef __LIST_H
#define __LIST_H

#include <string>


namespace cm_list {

template<class T>
struct node {

    T data;
    node<T> *next = nullptr;

    node() {}

    node(const T& _data, node<T> *_next = nullptr):
     data(_data), next(_next) { }
    
    // access next node
    node<T> *next_node() { return next; }

    // insert node p after this node
    void insert_next(node<T> *p) {
        p->next = next;
        next = p;
    }

    // remove node after this node and return removed node
    node<T> *remove_next() {
        node<T> *temp = next;
        if(nullptr != next) {
            next = next->next;
        }
        return temp;
    }

    // starting at this node, find next matching node
    node<T> *find_next(const T& _data) {
        node<T> *p = this;
        while(nullptr != p) {
            if(p->data == _data) {
                break;
            }
            p = p->next;
        }
        return p;
    }

    // return node at index from this node
    node<T> *index_node(int index) {
        node<T> *p = this;
        for(int j = 0; j < index && nullptr != p; j++) {
            p = p->next;
        }
        return p;
    }

};

template<class T>
node<T> *new_node(const T _data, node<T> *_next = nullptr) {
    node<T> *newnode = new node<T>(_data, _next);
    return newnode;
}

// free all nodes including head node
template<class T>
void free_all(node<T> **head) {
    node<T> *hp = *head;
    while(hp->next_node() != nullptr) {
        node<T> *np = hp->remove_next();
        if(nullptr != np) { 
            delete np;
            np = nullptr;
        }
    }
    delete hp;
    *head = nullptr;
}

template<class T>
bool detect_loop(node<T> *head, int *sz, node<T> **start) {

    // use Floyd's Tortois & Hare Cycle Detection algorithm
    // to detect and locate loop in the list...

    node<T> *tortoise = head;
    node<T> *hare = head;

    // first we detect the loop
    // hare moves two steps per iteration
    // tortoise moves one step per iteration
    bool loop_found = false;
    while(1) {

        // if hare reached the end
        if(hare == nullptr) {
            // no loop found
            break;
        }

        hare = hare->next;

        // if hare reached the end
        if(hare == nullptr) {
            // no loop found
            break;
        }

        hare = hare->next;
        tortoise = tortoise->next;

        if(hare == tortoise) {
            // loop found
            loop_found = true;
            break;
        }
    }

    // loop found, we determine the length of the loop
    // and where the loop begins...
    
    int len = 0;

    if(nullptr != start) *start = nullptr;
    if(nullptr != sz) *sz = 0;

    if(loop_found) {
        // tortoise walks to hare, counting steps
        // as it moves
        do {
            tortoise = tortoise->next;
            len++;
        } while( tortoise != hare);

        if(nullptr != sz) *sz = len;

        // move tortoise to start, walk both one
        // step until they meet again
        // they will meet where the loop begins
        tortoise = head;
        while(tortoise != hare) {
            tortoise = tortoise->next;
            hare = hare->next;
        }

        if(nullptr != start) *start = tortoise;
    }

    return loop_found;
}


} // namespace cm_list


#endif	// __LIST_H

