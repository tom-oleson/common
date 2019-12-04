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

    cm_list::node<char> *find = head->find_next('C');

    CPPUNIT_ASSERT(find->data == 'C');

    cm_list::free_all(&head);

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
        while( tortoise != hare) {
            tortoise = tortoise->next;
            hare = hare->next;
        }
    }

    CPPUNIT_ASSERT(loop_found == true);
    CPPUNIT_ASSERT(len > 0);
    CPPUNIT_ASSERT( tortoise->data == 4);
    CPPUNIT_ASSERT( hare->data == 4);

    
    cm_list::node<int> *head = p[0];

    //cm_list::free_all(&head);
}

void listTest::test_find_route_loop() { 

// create a list of linked nodes
    cm_list::node<std::string> *head, *input;
    cm_list::node<std::string> *lp, *rp;

    input = cm_list::new_node(std::string("etok"),
            cm_list::new_node(std::string("red"),
            cm_list::new_node(std::string("pub"),
            cm_list::new_node(std::string("blue"),
            cm_list::new_node(std::string("xray")) ))));

    // setup the loop we will detect and measure...
    
    head = input->find_next(std::string("etok"));
    head->index_node(0)->next = input->find_next(std::string("pub"));
    head->index_node(1)->next = input->find_next(std::string("xray"));
    head->index_node(2)->next = input->find_next(std::string("etok"));

    CPPUNIT_ASSERT( head->index_node(0)->data == std::string("etok"));
    CPPUNIT_ASSERT( head->index_node(1)->data == std::string("pub"));
    CPPUNIT_ASSERT( head->index_node(2)->data == std::string("xray"));
    CPPUNIT_ASSERT( head->index_node(3)->data == std::string("etok"));

    // find the loop and verify...
    int size = 0;
    cm_list::node<std::string> *start;

    bool found_loop = cm_list::detect_loop(head, &size, &start);

    CPPUNIT_ASSERT(found_loop == true); // loop found
    CPPUNIT_ASSERT(size == 3);  // loop size
    CPPUNIT_ASSERT(start->data == "etok");  // loop start node
    CPPUNIT_ASSERT(start->index_node(size-1)->data == "xray"); // loop end node

    //cm_list::free_all(&input);
}


bool loop_analysis(std::vector<std::pair<std::string, std::string>> &input) {

    // flatten key/value pairs into route strings
    // examples:
    // 12/03/2019 18:21:42 [info]: etok xray pub etok
    // 12/03/2019 18:21:42 [info]: etok red
    // 12/03/2019 18:21:42 [info]: etok pub etok
    // 12/03/2019 18:21:42 [info]: xray pub etok
    // 12/03/2019 18:21:42 [info]: pub etok xray pub etok    

    std:vector<std::string> route[input.size()];
    int index = 0;

    // for each input 
    for(auto it = input.begin(); it != input.end(); ++it) {

        std::string in_key = it->first;
        std::string in_pub = it->second;

        route[index].push_back(in_key);
        route[index].push_back(in_pub);

        // look for key that matches in_pub
        for(auto pt = input.begin(); pt != input.end(); ++pt) {
            std::string key = pt->first;
            std::string pub = pt->second;
            if(key == in_pub) {
                route[index].push_back(pub);
                // new search term forward
                in_pub = pub;
            }
         }
         index++;
    }

    // count the number of times each key occurs in the route
    // more than once means it is a loop in the route

    bool found_loop = false;

    for(int k = 0; k < input.size(); k++) {
        std::string s;
        std::map<std::string,int> counts;

        // build output log string and key count
        for(auto it = route[k].begin(); it != route[k].end(); ++it) {
            s += (*it) + " ";
            counts[*it]++;
        }

        // sum the number of keys that occur more than once;
        // that is the total number of loops in the route
        int loop_sum = 0;
        for(auto &it : counts) {
            int count = it.second;
            if(count > 1) {
                loop_sum += (count - 1);
                found_loop = true;
            }
        }

        std::string loop_report = loop_sum > 0 ?
         cm_util::format("%d loop(s)", loop_sum) : "OK";

        std::string msg = cm_util::format("%s: %s", s.c_str(), 
            loop_report.c_str());

        if(loop_sum > 0) cm_log::error(msg.c_str());
        else cm_log::info(msg.c_str());
    }

    return found_loop == false;
}

void listTest::test_key_value_loop() { 

    // alternative loop detection using key counts

    cm_log::file_logger test_log("./log/test_key_value_loop.log");
    set_default_logger(&test_log);

    std::vector<std::pair<std::string, std::string>> input;

    input.push_back(std::make_pair("etok", "xray"));
    input.push_back(std::make_pair("etok", "red"));
    input.push_back(std::make_pair("etok", "pub"));
    input.push_back(std::make_pair("xray", "pub"));
    input.push_back(std::make_pair("pub", "etok"));

    // inputs:
    // (1) etok --> xray
    // (2) etok --> red
    // (3) etok --> pub
    // (4) xray --> pub
    // (5) pub --> etok

    // (1) etok --> xray --> pub --> etok
    // (3) etok --> pub --> etok

    bool passed = loop_analysis(input);

    CPPUNIT_ASSERT(passed == false);

}