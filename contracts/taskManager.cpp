// Copyright 2019 fabulus@omic.ch
//
// Licensed under the Apache License, Version 2.0 (the "License");
// http://www.apache.org/licenses/LICENSE-2.0

#include <eosio/eosio.hpp>
//#include <eosio/multi_index.hpp>
//#include <eosio/action.hpp>
//#include <eosio/transaction.hpp>
//#include <eosio/asset.hpp>
//#include <eosio/crypto.hpp>
//#include <eosio/time.hpp>

#include "taskmanager_constants.hpp"

using namespace eosio;

using std::string;
using std::to_string;

class [[eosio::contract]] taskManager : public contract {

public:
    using contract::contract;

    taskManager(name self, name code, datastream<const char *> ds) :
            contract(self, code, ds),
            _tasks(self, self.value) {}

    [[eosio::action]]
    void setemployer(name account, string contact_name, string email) {
        require_auth(account);
        check(contact_name.length() > 0, "Contact name cannot be empty");
        check(email.length() > 0, "Email cannot be empty");

        auto setter = [&](auto &item) {
            item.account = account;
            item.contact_name = contact_name;
            item.email = email;
            item.is_active = 1;
        };

        employers _employers(_self, _self.value);

        auto employerIterator = _employers.find(account.value);
        if (employerIterator == _employers.end()) {
            _employers.emplace(account, setter);
        } else {
            _employers.modify(*employerIterator, account, setter);
        }
    }

    /**
     * @param employer
     * @param worker
     * @param description
     */
    [[eosio::action]]
    void newtask(name employer, name worker, string description) {
        require_auth(employer);
        check(description.length() > 0, "description cannot be empty");

        // Contract checks that worker account exists
        check(is_account(worker), "worker account does not exist");

        // tasks _tasks(_self, _self.value);

        // Contract checks that there’s no existing task from Alice to Bob
        // todo this is not what we want here.... we want to look by employer
        auto taskIterator = _tasks.find(worker.value);
        check(taskIterator != _tasks.end(), "This worker has an existing task");
        const task& task = *taskIterator;

        // Add the tasks to the worker
        auto setter = [&](auto &item) {
            item.employer = employer;
            item.worker = worker;
            item.description = description;
            item.is_completed = 0;
        };

        _tasks.emplace(employer, setter);

        // Contract notifies Bob
        _notify(name("newTaskAssigned"), "New task assigned by Employer ", worker);
    }

    /**
     * @param employer
     * @param worker
     */
    [[eosio::action]]
    void finished(name employer, name worker) {
        require_auth(employer);

        // The contract checks that such task exists
        auto taskIterator = _tasks.find(worker.value);
        check(taskIterator == _tasks.end(), "This worker has no task assigned");
        const task& t = *taskIterator;

        // The contract allows only Worker to execute this
        // todo...

        // Contract deletes the entry from the tasks table
        // todo...

        // Contract sends a notification to Alice
//        _notify(name("taskDone") ,
//                "Task has been completed by " + worker.value,
//                employer);
    }

private:

    struct [[eosio::table("tasks")]] task {
        uint64_t id;
        name employer;
        name worker;
        string description;
        uint8_t is_completed;

        auto primary_key() const { return id; }

        uint64_t get_employer() const { return employer.value; }
    };

    typedef eosio::multi_index<name("tasks"), task> tasks;


    struct [[eosio::table("employers")]] employer {
        name account;
        string contact_name;
        string email;
        uint8_t is_active;

        auto primary_key() const { return account.value; }

        uint64_t get_is_active() const { return is_active; }
    };

    typedef eosio::multi_index<name("employers"), employer> employers;


//    typedef eosio::multi_index<
//            name("employers"), employer,
//            indexed_by<name("active"), const_mem_fun<employer, uint64_t, &employer::get_is_active>>>
//    employers;

private:

    tasks _tasks;


};