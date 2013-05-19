#!/bin/bash
kill `ps a | egrep [g]br2 | awk '{print $1}'`

