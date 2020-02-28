/*
   Copyright (c) 2020 Stefan Kremser (@Spacehuhn)
   This software is licensed under the MIT License. See the license file for details.
   Source: github.com/spacehuhn/esp8266_deauther
 */

#include "TargetList.h"

#include <string.h>
#include <stdlib.h>

// ========== Target =========== //
Target::Target(const uint8_t* from, const uint8_t* to, uint8_t ch) {
    memcpy(this->from, from, 6);
    memcpy(this->to, to, 6);
    this->ch   = ch;
    this->next = NULL;
}

const uint8_t* Target::getFrom() const {
    return from;
}

const uint8_t* Target::getTo() const {
    return to;
}

uint8_t Target::getCh() const {
    return ch;
}

Target* Target::getNext() {
    return this->next;
}

void Target::setNext(Target* next) {
    this->next = next;
}

bool Target::operator==(const Target& t) const {
    return memcmp(from, t.from, 6) == 0 &&
           memcmp(to, t.to, 6) == 0 &&
           ch == t.ch;
}

bool Target::operator<(const Target& t) const {
    return memcmp(from, t.from, 6) < 0 ||
           memcmp(to, t.to, 6) < 0 ||
           ch < t.ch;
}

bool Target::operator>(const Target& t) const {
    return memcmp(from, t.from, 6) > 0 &&
           memcmp(to, t.to, 6) > 0 &&
           ch > t.ch;
}

// ========== TargetList =========== //
TargetList::TargetList(int max) : list_max_size(max) {}

TargetList::~TargetList() {
    clear();
}

void TargetList::moveFrom(TargetList& t) {
    Target* tmp = t.list_begin;

    while (tmp) {
        if ((list_max_size > 0) && (list_size >= list_max_size)) break;

        // Push to list
        if (!list_begin) {
            list_begin = tmp;
            list_end   = tmp;
            list_h     = list_begin;
        } else {
            list_end->setNext(tmp);
            list_end = tmp;
        }

        ++(list_size);

        tmp = tmp->getNext();
    }

    t.list_begin = NULL;
    t.list_end   = NULL;
    t.list_size  = 0;
    t.list_h     = NULL;
    t.list_pos   = 0;
}

bool TargetList::push(const uint8_t* from, const uint8_t* to, const uint8_t ch) {
    if ((list_max_size > 0) && (list_size >= list_max_size)) return false;

    // Create new target
    Target* new_target = new Target(from, to, ch);

    // Empty list -> insert first element
    if (!list_begin) {
        list_begin = new_target;
        list_end   = new_target;
        list_h     = list_begin;
    } else {
        // Insert at start
        if (list_begin > new_target) {
            new_target->setNext(list_begin);
            list_begin = new_target;
        }
        // Insert at end
        else if (list_end < new_target) {
            list_end->setNext(new_target);
            list_end = new_target;
        }
        // Insert somewhere in the between (insertion sort)
        else {
            Target* tmp_c = list_begin;
            Target* tmp_p = NULL;

            do {
                tmp_p = tmp_c;
                tmp_c = tmp_c->getNext();
            } while (tmp_c && tmp_c < new_target);

            // Skip duplicates
            if (tmp_c == new_target) {
                free(new_target);
                return false;
            } else {
                new_target->setNext(tmp_c);
                if (tmp_p) tmp_p->setNext(new_target);
            }
        }
    }

    ++(list_size);
    return true;
}

Target* TargetList::get(int i) {
    if (i < list_pos) begin();

    while (list_h && list_pos<i) iterate();

    return list_h;
}

void TargetList::begin() {
    list_h   = list_begin;
    list_pos = 0;
}

Target* TargetList::iterate() {
    Target* tmp = list_h;

    if (list_h) {
        list_h = list_h->getNext();
        ++list_pos;
    }

    return tmp;
}

bool TargetList::available() const {
    return list_h;
}

int TargetList::size() const {
    return list_size;
}

bool TargetList::full() const {
    return list_max_size > 0 && list_size >= list_max_size;
}

void TargetList::clear() {
    Target* tmp = list_begin;

    while (tmp) {
        Target* to_delete = tmp;
        tmp = tmp->getNext();
        delete tmp;
    }

    list_begin = NULL;
    list_end   = NULL;
    list_size  = 0;

    list_h   = NULL;
    list_pos = 0;
}