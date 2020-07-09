/*
 * Copyright (C) 2020 Open Source Robotics Foundation
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
*/
#include <gtest/gtest.h>
#include <ignition/common/Console.hh>
#include "test_config.h"

#include "ignition/gui/Enums.hh"
#include "ignition/gui/PlottingInterface.hh"

using namespace ignition;
using namespace gui;

TEST(PlottingInterfaceTest, Topic)
{
    common::Console::SetVerbosity(4);

    // ============== Register & UnRegister Test =============

    // prepare the msg
    msgs::Collision msg;
    auto pose = new ignition::msgs::Pose();
    auto vector3d = new ignition::msgs::Vector3d();
    vector3d->set_x(10);
    vector3d->set_z(15);
    pose->set_allocated_position(vector3d);
    msg.set_allocated_pose(pose);

    auto topic = Topic("");

    topic.Register("pose-position-x", 1);
    topic.Register("pose-position-x", 2);
    topic.Register("pose-position-y", 1);
    topic.Register("pose-position-y", 2);
    topic.UnRegister("pose-position-y", 2);

    auto fields = topic.Fields();
    ASSERT_EQ(topic.FieldCount(), 2);

    // size test
    EXPECT_EQ(fields["pose-position-x"]->ChartCount(), 2);
    EXPECT_EQ(fields["pose-position-y"]->ChartCount(), 1);

    // charts test
    EXPECT_TRUE(fields["pose-position-x"]->Charts().find(1) !=
            fields["pose-position-x"]->Charts().end());
    EXPECT_TRUE(fields["pose-position-x"]->Charts().find(2) !=
            fields["pose-position-x"]->Charts().end());
    EXPECT_TRUE(fields["pose-position-y"]->Charts().find(1) !=
            fields["pose-position-x"]->Charts().end());

    // test the removing of the field if it has not attatched charts
    topic.UnRegister("pose-position-y",1);
    EXPECT_EQ(topic.FieldCount(), 1);


    // =========== Callback Test ============
    topic.Register("pose-position-z",1);

    // update the fields
    topic.Callback(msg);

    fields = topic.Fields();

    EXPECT_EQ(int(fields["pose-position-x"]->Value()), 10);
    EXPECT_EQ(int(fields["pose-position-z"]->Value()), 15);
}

TEST(PlottingInterfaceTest, Transport)
{
    // =========== Publish Test =================
    transport::Node node;

    auto pub = node.Advertise<msgs::Collision> ("/collision_topic");
    std::this_thread::sleep_for(std::chrono::milliseconds(200));

    auto transport = Transport();
    transport.Subscribe("/collision_topic", "pose-position-x", 1);
    transport.Subscribe("/collision_topic", "pose-position-z", 1);

    // prepare the msg
    msgs::Collision msg;
    auto pose = new ignition::msgs::Pose();
    auto vector3d = new ignition::msgs::Vector3d();
    vector3d->set_x(10);
    vector3d->set_z(15);
    pose->set_allocated_position(vector3d);
    msg.set_allocated_pose(pose);

    // publish to call the topic::Callback
    pub.Publish(msg);

    // wait for callback
    std::this_thread::sleep_for(std::chrono::milliseconds(50));

    auto topics = transport.Topics();

    EXPECT_EQ(topics["/collision_topic"]->FieldCount(), 2);

    auto fields = topics["/collision_topic"]->Fields();

    EXPECT_EQ(int(fields["pose-position-x"]->Value()), 10);
    EXPECT_EQ(int(fields["pose-position-z"]->Value()), 15);


    // =========== Many Topics Test =================
    // add another topic to the transport and subscribe to it
    node.Advertise<msgs::Int32> ("/test_topic");
    transport.Subscribe("/test_topic", "data", 2);

    topics = transport.Topics();

    ASSERT_EQ(int(topics.size()), 2);
    EXPECT_EQ(topics["/test_topic"]->FieldCount(), 1);


    // =========== UnSubscribe Test =================

    // test the deletion of the topic if it has no fields
    transport.Unsubscribe("/collision_topic", "pose-position-z", 1);
    transport.Unsubscribe("/collision_topic", "pose-position-x", 1);

    topics = transport.Topics();
    EXPECT_EQ(int(topics.size()), 1);
}

//// test class [ Under Progress ]
//class PlotTest : public QObject
//{
//    Q_OBJECT
//    public : PlotTest(PlottingInterface *plotIface)
//    {
//        connect(plotIface, SIGNAL(plot()), this, SLOT(plot()));
//    }
//    public slots: void plot(int _chart, QString _fieldID, double _x, double _y)
//    {
//        EXPECT_EQ(_chart, 1);
//        EXPECT_EQ(_fieldID, "data");
//        EXPECT_GE(int(_x), 1);
//        EXPECT_EQ(int(_y), 10);
//    }
//};


//TEST(PlottingInterfaceTest, PlottingIface)
//{

//    auto PlottingIface = new PlottingInterface();
//    auto plotTest = new PlotTest(PlottingIface);

//    transport::Node node;

//    auto pub = node.Advertise<msgs::Collision> ("/collision_topic");
//    std::this_thread::sleep_for(std::chrono::milliseconds(100));

//    PlottingIface->subscribe("/test_plotting_iface","data",1);

//    ignition::msgs::Int32 msg;
//    msg.set_data(10);

//    pub.Publish(msg);

//    // wait for callback
//    std::this_thread::sleep_for(std::chrono::milliseconds(50));

//    delete plotTest;
//    delete PlottingIface;
//}

