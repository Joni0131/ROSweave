- [ROSweave](#rosweave)
- [Origin](#origin)
- [Functionality](#functionality)
  - [WebServer](#webserver)
  - [OperationMode](#operationmode)
    - [Configurator](#configurator)
      - [Setup MicroRos](#setup-microros)
      - [Create Components](#create-components)
    - [Loop](#loop)
- [Extension](#extension)
  - [Desgin](#desgin)
  - [Webserver](#webserver-1)
  - [Component Class](#component-class)
  - [Generator](#generator)
- [MicroRos Agent](#microros-agent)

# ROSweave

ROSweave provides a in the field configuration of a esp32 microros node with predefined connected components.

# Origin

During my work on my rospCar repository i realized that the software archticture only allwoed one static esp configuration. 
I came to the conclusion so that other people can use this software as well that I have to make the whole system more configurable.

With this repo I aim to address those problems and also so I can better continue on my actual project.

# Functionality

The esp has two operating modes, the webserver mode and the operation mode, this allows even in the field to adjust the configuration. 

## WebServer

The Webserver is using a SPIFFS to store the configuration in a json style format on the esp. During the creation of the json file the user can also see its content so theoratically even building a whole simulation of the esp and its interfaces could be achievable.

On this webserver the user is selecting the Hardware components that he wants to connect and on what pins they are connected.

It is also not possible to assign the same pin to multiple devices

## OperationMode

During the operation mode the system has the following states

### Configurator

During this step the filesystem is prepared and the configuration from the webserver is parsed.

#### Setup MicroRos

First the Background MicroRos components are setup.

#### Create Components

Based on the configuration generator functions are called to create objects of the component type.

### Loop

During the loop all publisher, subscriber and services are spinning with a set frequency, that is much faster than the frequency of the publishers. 

# Extension

Extending the code with more components is quite straight forward.

## Desgin

Decide what is necessary for your component. Does it need publisher, subscriber, services? What is their cycle time? What pins and pin capabilities are necessary.

## Webserver

Create a new component in the webserver that generates the config in the normal style.

## Component Class

Create a new component class that implements the initiator and the necessary callback functions.

## Generator

Implement a new function that parses the config and generates a instance of the new class.

# MicroRos Agent

For the other side of the MicroRos architecture I prepared and adjusted a podman container so the messages and services from the extra packages of the platformio are taken and available in the ros2 environment.

