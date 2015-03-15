// Copyright (c) Microsoft Open Technologies, Inc. All rights reserved.
// Licensed under the Apache License, Version 2.0. See License.txt in the project root for license information.

using System;
using System.Collections.Generic;
using System.Threading.Tasks;
using Microsoft.AspNet.SignalR;

namespace SelfHost
{
    public class HubConnection : Hub
    {
        public IEnumerable<int> ForceReconnect()
        {
            yield return 1;
            // throwing here will close the websocket which should trigger reconnect
            throw new Exception();
        }

        public string DisplayMessage(string message)
        {
            Clients.Caller.displayMessage("Send: " + message);
            return message;
        }

        public void Send()
        {
            Clients.Caller.send("Send");
        }
        public int SendNumber(int num)
        {
            Clients.Caller.sendNumber(num);
            return num;
        }

        public int SendMessages(int num, string message)
        {
            Clients.Caller.sendMessages(num, message);
            return num;
        }

        public async Task<string> ReportProgress(string jobName, IProgress<int> progress)
        {
            for (int i = 0; i <= 10; i++)
            {
                await Task.Delay(250);
                progress.Report(i);
            }
            return String.Format("{0} done!", jobName);
        }

        public Person ComplexType(Person p)
        {
            Clients.Caller.displayPerson(p);
            return p;
        }

        public class Person
        {
            public string Name { get; set; }
            public int Age { get; set; }
            public Address Address { get; set; }
        }

        public class Address
        {
            public string Street { get; set; }
            public string Zip { get; set; }
        }
    }
}