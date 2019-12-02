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
