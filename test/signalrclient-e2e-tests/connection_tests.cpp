// Copyright (c) Microsoft Open Technologies, Inc. All rights reserved.
// Licensed under the Apache License, Version 2.0. See License.txt in the project root for license information.

#pragma once

#include "stdafx.h"
#include <string>
#include "cpprest\details\basic_types.h"
#include "cpprest\json.h" 
#include "connection.h"
#include "hub_connection.h"
#include "url.h"

using namespace signalr;
using namespace utility;
using namespace std;
using namespace pplx;

TEST(connection_tests, send_message)
{
    auto conn = make_shared<connection>(url + U("raw-connection"));
    auto message = make_shared<string_t>();
    auto test_event = make_shared<pplx::event>();

    conn->set_message_received([message, test_event](const string_t& payload)
    {
        *message = payload;
        test_event->set();
    });

    conn->start().then([conn](task<void> start_task)
    {
        try
        {
            start_task.get();
            ASSERT_EQ(conn->get_connection_state(), connection_state::connected);

            web::json::value obj;
            obj[U("type")] = web::json::value::number(0);
            obj[U("value")] = web::json::value::string(U("test"));

            conn->send(obj.serialize());
        }
        catch (...)
        { }
    });

    ASSERT_FALSE(test_event->wait(10000));

    ASSERT_EQ(*message, U("\"Send: test\""));
}

TEST(connection_tests, send_message_after_connection_restart)
{
    auto conn = make_shared<connection>(url + U("raw-connection"));
    auto message = make_shared<string_t>();
    auto test_event = make_shared<pplx::event>();

    conn->set_message_received([message, test_event](const string_t& payload)
    {
        *message = payload;
        test_event->set();
    });

    conn->start().then([conn](task<void> start_task)
    {
        try
        {
            start_task.get();
            ASSERT_EQ(conn->get_connection_state(), connection_state::connected);
        }
        catch (...)
        {
            ASSERT_TRUE(false);
        }
    }).get();

    conn->stop().then([conn](task<void> stop_task)
    {
        try
        {
            stop_task.get();
            ASSERT_EQ(conn->get_connection_state(), connection_state::disconnected);
        }
        catch (...)
        {
            ASSERT_TRUE(false);
        }
    }).get();

    conn->start().then([conn](task<void> start_task)
    {
        try
        {
            start_task.get();
            ASSERT_EQ(conn->get_connection_state(), connection_state::connected);

            web::json::value obj;
            obj[U("type")] = web::json::value::number(0);
            obj[U("value")] = web::json::value::string(U("test"));

            conn->send(obj.serialize());
        }
        catch (...)
        { }
    });

    ASSERT_FALSE(test_event->wait(15000));

    ASSERT_EQ(*message, U("\"Send: test\""));
}

