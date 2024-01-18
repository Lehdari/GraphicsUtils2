#!/bin/bash

if ! [ -f main_sponza.zip ]; then
  wget https://cdrdv2.intel.com/v1/dl/getContent/726594 -O main_sponza.zip || rm -f main_sponza.zip
fi

if [ -f main_sponza.zip ]; then
  if unzip main_sponza
  then
    rm -f main_sponza.zip
  fi
fi
