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

TEST(hub_connection_tests, send_message)
{
    auto hubConn = make_shared<hub_connection>(url + U("SignalR"));
    auto message = make_shared<string_t>();
    auto received_event = make_shared<pplx::event>();
    auto test_event = make_shared<pplx::event>();

    auto hubProxy = hubConn->create_hub_proxy(U("hubConnection"));

    hubProxy.on(U("displayMessage"), [message, received_event](const web::json::value& arguments)
    {
        *message = arguments.serialize();
        received_event->set();
    });

    hubConn->start().then([hubConn, &hubProxy, test_event](task<void> start_task)
    {
        try
        {
            start_task.get();
            ASSERT_TRUE(hubConn->get_connection_state() == connection_state::connected);

            web::json::value obj{};
            obj[0] = web::json::value(U("test"));

            hubProxy.invoke<web::json::value>(U("displayMessage"), obj)
                .then([test_event](task<web::json::value> send_task)
            {
                try
                {
                    auto test = send_task.get();
                    ASSERT_EQ(test.serialize(), U("\"test\""));
                    test_event->set();
                }
                catch (...)
                { }
            });
        }
        catch (...)
        { }
    });

    ASSERT_FALSE(received_event->wait(10000));

    ASSERT_FALSE(test_event->wait(10000));

    ASSERT_EQ(*message, U("[\"Send: test\"]"));
}

TEST(hub_connection_tests, send_message_after_connection_restart)
{
    auto hubConn = make_shared<hub_connection>(url + U("SignalR"));
    auto message = make_shared<string_t>();
    auto received_event = make_shared<pplx::event>();
    auto test_event = make_shared<pplx::event>();

    auto hubProxy = hubConn->create_hub_proxy(U("hubConnection"));

    hubProxy.on(U("displayMessage"), [message, received_event](const web::json::value& arguments)
    {
        *message = arguments.serialize();
        received_event->set();
    });

    hubConn->start().then([hubConn](task<void> start_task)
    {
        try
        {
            start_task.get();
            ASSERT_EQ(hubConn->get_connection_state(), connection_state::connected);
        }
        catch (...)
        { }
    }).then([]()
    {
        return;
    });

    hubConn->stop().then([hubConn](task<void> stop_task)
    {
        try
        {
            stop_task.get();
            ASSERT_EQ(hubConn->get_connection_state(), connection_state::disconnected);
        }
        catch (...)
        { }
    }).then([]()
    {
        return;
    });

    hubConn->start().then([hubConn, &hubProxy, test_event](task<void> start_task)
    {
        try
        {
            start_task.get();
            ASSERT_EQ(hubConn->get_connection_state(), connection_state::connected);

            web::json::value obj{};
            obj[0] = web::json::value(U("test"));

            hubProxy.invoke<web::json::value>(U("displayMessage"), obj)
                .then([test_event](task<web::json::value> send_task)
            {
                try
                {
                    auto test = send_task.get();
                    ASSERT_EQ(test.serialize(), U("\"test\""));
                    test_event->set();
                }
                catch (...)
                { }
            });
        }
        catch (...)
        { }
    });

    ASSERT_FALSE(received_event->wait(15000));

    ASSERT_FALSE(test_event->wait(15000));

    ASSERT_EQ(*message, U("[\"Send: test\"]"));
}

TEST(hub_connection_tests, send_message_after_reconnect)
{
    auto hubConn = make_shared<hub_connection>(url + U("SignalR"));
    auto message = make_shared<string_t>();
    auto reconnecting_event = make_shared<pplx::event>();
    auto reconnected_event = make_shared<pplx::event>();
    auto received_event = make_shared<pplx::event>();

    hub_proxy hubProxy = hubConn->create_hub_proxy(U("hubConnection"));

    hubConn->set_reconnecting([hubConn, reconnecting_event]()
    {
        ASSERT_EQ(hubConn->get_connection_state(), connection_state::reconnecting);
        reconnecting_event->set();
    });

    hubConn->set_reconnected([hubConn, reconnected_event]()
    {
        ASSERT_EQ(hubConn->get_connection_state(), connection_state::connected);
        reconnected_event->set();
    });

    hubProxy.on(U("displayMessage"), [message, received_event](const web::json::value& arguments)
    {
        *message = arguments.serialize();
        received_event->set();
    });

    hubConn->start().then([hubConn, &hubProxy](task<void> start_task)
    {
        try
        {
            start_task.get();
            ASSERT_EQ(hubConn->get_connection_state(), connection_state::connected);

            web::json::value obj{};
            obj[0] = web::json::value(U("test"));

            hubProxy.invoke<web::json::value>(U("forceReconnect"))
                .then([](pplx::task<web::json::value> send_task)
            {
                try
                {
                   send_task.get();
                }
                catch (...)
                { }
            });
        }
        catch (...)
        { }
    });

    ASSERT_FALSE(reconnecting_event->wait(60000));

    ASSERT_FALSE(reconnected_event->wait(60000));

    auto test_event = make_shared<pplx::event>();

    web::json::value obj{};
    obj[0] = web::json::value(U("test"));

    hubProxy.invoke<web::json::value>(U("displayMessage"), obj)
        .then([test_event](task<web::json::value> send_task)
    {
        auto test = send_task.get();
        ASSERT_EQ(test.serialize(), U("\"test\""));
        test_event->set();
    });

    ASSERT_FALSE(received_event->wait(10000));

    ASSERT_FALSE(test_event->wait(10000));

    ASSERT_EQ(*message, U("[\"Send: test\"]"));
}