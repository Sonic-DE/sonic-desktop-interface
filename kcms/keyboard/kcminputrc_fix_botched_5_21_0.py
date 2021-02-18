#
#    SPDX-FileCopyrightText: 2021 Janet Blackquill <uhhadd@gmail.com>
#
#    SPDX-License-Identifier: LGPL-2.0-or-later
#

#!/usr/bin/env python
import sys

content = sys.stdin.read()

# 5.21.0 botched the migration by accidentally turning enabled to disabled and disabled to enabled
# so this script checks if the botched migration was applied, and if so, reverses it.
if "kcminputrc_repeat.upd:kcminputrc_migrate_repeat_value" in content:
    print("# DELETE KeyRepeat")

    if "KeyRepeat=repeat" in content:
        print("KeyRepeat=nothing")
    elif "KeyRepeat=nothing" in content:
        print("KeyRepeat=repeat")
