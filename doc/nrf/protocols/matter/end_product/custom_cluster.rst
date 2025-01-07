.. _ug_matter_custom_cluster:

Custom Matter cluster
#####################

.. contents::
   :local:
   :depth: 2

This document serves as a comprehensive guide for developers looking to understand and utilize the Matter Interface Definition Language (IDL) to create custom Matter clusters.
The primary goal is to explain the fundamentals of Matter IDL, and provide a step-by-step tutorial on how to define and implement a new user-specific Matter cluster.
Additionally, it will cover the process of integrating the newly created cluster into the common Matter database, ensuring seamless interoperability and functionality within the Matter ecosystem.

Matter IDL
**********

Matter IDL is a language used to define the interfaces and data structures for Matter devices.
It allows developers to describe the capabilities and attributes of a device in a standardized way, which can then be used to generate code and documentation for different platforms and languages.

The IDL file typically includes definitions for:
- Clusters: Groups of attributes and commands that represent a specific functionality.
- Attributes: Data points that can be read or written.
- Commands: Actions that can be invoked on the device.

A Matter IDL file is defined as a :ref:`Lark <https://lark-parser.readthedocs.io/en/latest/index.html>` language grammar, which is a Python library for parsing context-free grammars.
Thanks to that the IDL file is intended for human readability as well as machine parsing.

How to create a custom cluster
******************************

To create a custom cluster and add it to the Matter database you need to create a Matter IDL file, use the generator to covert it to the XML format and then append it to existing Matter database.
The guide has been divided into the three steps to make it easier to follow.
In this guide the simple cluster will be created with one attribute and one command.

Create a Matter IDL file
========================

To create Matter IDL file you need to define the cluster, attributes, and commands.
You can follow a guidance from the :ref:`Matter IDL documentation <>` to see an example on how to write a Matter IDL file.

The following example shows how to create a custom cluster with one attribute and one command.
Each step is described as a comment so you can copy it directly to your :file:`.matter` file and use for the next steps.

