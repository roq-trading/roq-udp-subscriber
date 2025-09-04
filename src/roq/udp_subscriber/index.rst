.. _roq-udp-subscriber:

roq-udp-subscriber
==================

UDP (multicast) subscriber

The subscriber allows you receiver :cpp:class:`CustomMetrics` over UDP (multicast).


Installing
----------

* :ref:`Using Conda <tutorial-conda>`

.. tab:: Unstable

  .. code-block:: shell

     $ conda install \
           --channel https://roq-trading.com/conda/unstable \
           roq-udp-subscriber

.. tab:: Stable

  .. code-block:: shell

     $ conda install \
           --channel https://roq-trading.com/conda/stable \
           roq-udp-subscriber


Using
-----

.. code-block:: shell

   $ roq-udp-subscriber \
         --name "udp-subscriber" \
         --config_file $CONFIG_FILE_PATH \
         --client_listen_address $UNIX_SOCKET_PATH \
         --flagfile $ENVIRONMENT_FLAGFILE


.. _roq-udp-subscriber-flags:

Flags
-----

* :ref:`Using Flags <abseil-cpp>`
* :ref:`Gateway Flags <gateway-flags>`

.. code-block:: bash

   $ roq-udp-subscriber --help

.. tab:: Flags

   .. include:: flags/flags.rstinc

.. tab:: Misc

   .. include:: flags/misc.rstinc


Configuration
-------------

* :ref:`Gateway Config <gateway-config>`

.. code-block:: shell

   $ $CONDA_PREFIX/share/roq-udp-subscriber/config.toml

.. important::

   The template will be replaced when the software is upgraded.
   Make a copy and modify to your needs.

.. include:: config.toml
   :code: toml


Constraints
-----------
