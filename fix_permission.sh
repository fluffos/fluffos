#!/bin/bash

find . -type d -exec chmod 0755 {} +
find . -type f -exec chmod 0644 {} +
find . -type f -name "*.sh" -exec chmod 0755 {} +
find . -type f -name "*.py" -exec chmod 0755 {} +
chmod +x ./compat/fixstatic
