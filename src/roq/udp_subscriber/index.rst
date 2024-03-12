.. _roq-udp-subscriber:

roq-udp-subscriber
==================


Purpose
-------

* UDP (multicast) subscriber


Description
-----------

The subscriber allows you receiver :cpp:class:`CustomMetrics` over UDP (multicast).


Conda
-----

* :ref:`Using Conda <tutorial-conda>`

.. tab:: Install
  
  .. code-block:: bash
  
    $ mamba install \
      --channel https://roq-trading.com/conda/stable \
      roq-udp-subscriber
  
.. tab:: Configure

  .. code-block:: bash
  
    $ cp $CONDA_PREFIX/share/roq-udp-subscriber/config.toml $CONFIG_FILE_PATH
  
    # Then modify $CONFIG_FILE_PATH to match your specific configuration
  
.. tab:: Run
  
  .. code-block:: bash
  
    $ roq-udp-subscriber \
          --name "udp-subscriber" \
          --config_file "$CONFIG_FILE_PATH" \
          --service_listen_address "$TCP_LISTEN_PORT_FOR_METRICS" \
          --listen_address "$TCP_LISTEN_PORT_FOR_WS_CLIENTS" \
          --flagfile "$FLAG_FILE"
  

Config
------

.. tab:: Users
  
  A list of clients allowed to connect to the subscriber.

  .. code-block:: toml

    [users]
   
     [users.MD1]
     username="tbmd1"


Flags
-----

* :ref:`Using Flags <abseil-cpp>`

.. code-block:: bash

   $ roq-udp-subscriber --help

.. tab:: Flags

   .. include:: flags/flags.rstinc

.. tab:: Common

   .. include:: flags/common.rstinc


Constraints
-----------
