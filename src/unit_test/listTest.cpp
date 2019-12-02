/**********************************************************************
*
* listTest.cpp
*
**********************************************************************/

#include <cppunit/config/SourcePrefix.h>

#include <iostream>	// for cout

#include "listTest.h"

CPPUNIT_TEST_SUITE_REGISTRATION( listTest );

//void listTest::setUp() { }
//void listTest::tearDown() { }


void listTest::test_list() {

    cm_list::node<char> *p, *q, *r;

    q = new cm_list::node<char>('B');
    p = new cm_list::node<char>('A', q);
    r = new cm_list::node<char>('C');

    q->insert_next(r);

    CPPUNIT_ASSERT( p->data == 'A');
    CPPUNIT_ASSERT( p->next_node()->data == 'B');
    CPPUNIT_ASSERT( q->data == 'B');
    CPPUNIT_ASSERT( q->next_node()->data == 'C');
    CPPUNIT_ASSERT( r->data == 'C');

    cm_list::node<char> *p_next = p->next_node();

    CPPUNIT_ASSERT( p_next->data == 'B');

    r = q->remove_next();

    CPPUNIT_ASSERT( r->data == 'C');

    // create a chain of nodes in one shot...
    cm_list::node<char> *head =
        cm_list::new_node('A',
        cm_list::new_node('B',
        cm_list::new_node('C')));

    CPPUNIT_ASSERT(head->data == 'A');
    CPPUNIT_ASSERT(head->next->data == 'B');
    CPPUNIT_ASSERT(head->next->next->data == 'C');

    while(head->next_node() != nullptr) {
        cm_list::node<char> *np = head->remove_next();
        delete np;
    }

    delete r;
    delete p;
    delete q;

}

void listTest::test_floyds_algorithm() { 

    // cycle finding algorithm

    // create a list of linked nodes
    cm_list::node<int> *p[10];

    for(int j = 0; j < 10; j++) {
        p[j] = cm_list::new_node(j);
        if(j > 0) p[j-1]->next = p[j];
    }
        
    CPPUNIT_ASSERT(p[0]->data == 0);
    CPPUNIT_ASSERT(p[4]->data == 4);
    CPPUNIT_ASSERT(p[9]->data == 9);

    // setup a loop to detect

    // point the tail to the center
    p[9]->next = p[4];

    // begin the algorithm

    cm_list::node<int> *tortoise = p[0];
    cm_list::node<int> *hare = p[0];

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
    if(loop_found) {
        // tortoise walks to hare, counting steps
        // as it moves
        do {
            tortoise = tortoise->next;
            len++;
        } while( tortoise != hare);


        // move tortoise to start, walk both one
        // step until they meet again
        // they will meet where the loop begins
        tortoise = p[0];
        do {
            tortoise = tortoise->next;
            hare = hare->next;
        } while( tortoise != hare);
    }

    CPPUNIT_ASSERT(loop_found == true);
    CPPUNIT_ASSERT(len > 0);
    CPPUNIT_ASSERT( tortoise->data == 4);
    CPPUNIT_ASSERT( hare->data == 4);

}