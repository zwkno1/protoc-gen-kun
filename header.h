#pragma once

#include <protobuf.h>

void GenerateHeader(Printer& p)
{
    p.Print(R"cc(
/***************************************************************************************************
::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
:::::::::::::::::::::::::::::::::::::::::::::::::::::::...::::::::::::::::::::::::::::::::::::::::::
::::::::::::::::::::::::::::::::::::::::::::::::::::..::::....::::::::::::::::::::::::::::::::::::::
::::::::::::::::::::::::::::::::::::::::::::::::::::^!?YYJ?7!^..::::::::::::::::::::::::::::::::::::
::::::::::::::::::::::::::::::::::::::::::::::::::^7Y5PPPPPP5Y!:::::::::::::::::::::::::::::::::::::
::::::::::::::::::::::::::::::::::::::::::........^YPGGGGGPPGGP!::::::::::::::::::::::::::::::::::::
:::::::::::::::::::::::::::::::::::::::...:^!JYY^^JGBBGBBG?JPGP!::::::::::::::::::::::::::::::::::::
::::::::::::::::::::::::::::::::::::::.:!YPGBBBBP!~PBBBBGB57??!:::::::::::::::::::::::::::::::::::::
::::::::::::::::::::::::::::::::::::..^YBBBBBBBBBB?^5BBBBBBP7:.:::::::::::::::::::::::::::::::::::::
:::::::::::::::::::::::::::::::::::.:!P#BBBBBBBBBBB?^P#BBBGGY^.:::::::::::::::::::::::::::::::::::::
:::::::::::::::::::::::::::::::::..~5B#B###BBBBBBBBB~!GY7~!7J7:.::::::::::::::::::::::::::::::::::::
::::::::::::::::::::::::::::::::.:?B###&#Y5BBBBBBB##Y!!~?PB#BBG7::::::::::::::::::::::::::::::::::::
^::::::::::::::::::::::::::::::.^5####&#J:?#BBBB####575B#######Y:.::::::::::::::::::::::::::::::::::
^^^::..:::::::::::::::::::::::.:Y&###&&J.!G#BB#####B7~B########5:.:.::::::::::::::::::::::::::::::::
^^^~^^::.::::::::::::::::::::::.!P#&###?7B#BB######G~J########&#?::.::::::::::::::::::::::::::::::::
^^~~~~^^::.:::::::::::::::::::::.:!P###BB#BBB######Y^P#########&G^.:::::::::::::::::::::::::::::::::
^~~~~~~~~^:::::::::::::::::::::::..:?G###BBB#######7~B#########&G^.:::::::::::::::::::::::::::::::::
~~~~~~~~~~~^^:::.::::::::::::::::::..:?PB#########B7J###########5:::::::::::::::::::::::::::::::::::
~~~~~~~~~~~~~~^:::.::::::::::::::::::..^7555PGBBB#GYG##########B~.::::::::::::::::::::::::::::::::::
~~~~~~~~~~~~~~~~^^::::::::::::::::::::7J?JJJJJJYYYYJYY5PGBB#&&&Y::::::::::::::::::::::::::::::::::::
~~~~~~~~~~~~~~~~~~^^:::.::::::::::::.~YJ?JJJJJJJYYYYYYJYYY55??!:::::::::::::::::::::::::::::::::::::
^~~~~~~~~~~~~~~~~~~~~^:::::::::::::.:7J??JJJJJJJYYYYYJYYYYYJ^..:::::::::::::::::::::::::::::::::::::
:^^~~~~~~~~~~~~~~~~~~~~^^::::::::::~7??JJJJJYYYYYYYYYYYYYYYY7^..::::::::::::::::::::::::::::::::::::
::::^~~~~~~~~~~~~~~~~~~~~^^:::::::!????JJJJYYYYYY55YYYYYYYYJJ?!:::::::::::::::::::::::::::::::::::::
::::::^~~~~~~~~~~~~~~~~~~~~~^^::~7??JJJJYYYY5555555555555YYJJJJ7::::::::::::::::::::::::::::::::::::
:::::::::^~~~~~~~~~~~~~~~~~~~~~!?JJJJJJYYY555J?!^^!?Y55555YYYJJ?^:::::::::::::::::::::::::::::::::::
:::::::::::^~~~~~~~~~~~~~~~~~~7?JJJJYYY555J7^:......^!Y555YYYYJJ?^::::::::::::::::::::::::::::::::::
:::::::::::::^^~~~~~~~~~~~~~~7?JJJJJY55Y7~:..::::::::.^?55YYYYYJJ7^:::::::::::::::::::::::::::::::::
:::::::::::::::^^~~~~~~~~~~~7JJYYYY55Y?^:.::::::::::::.:!Y5555YYYY7:::::::::::::::::::::::::::::::::
::::::::::::::::::^~~~~~~~~~7??J5PP5J!~~^^::::::::::::::.:!YP555YYJ!::::::::::::::::::::::::::::::::
::::::::::::::::::::^^~~~~~77?JJY5J!~~~~~~~^::::::::::::::.^7Y555YJ?7:::::::::::::::::::::::::::::::
:::::::::::::::::::::::^~~~7???JYJ!~~~~~~~~~~^::::::::::::::.:7YYJ???!::::::::::::::::::::::::::::::
~~~~~~~~~~~~~~~~~~~~~~~~~~!7???JY?!!!!!!~~~~~~~^^^^^^^^^^^^^^^^7JJ????!^^^^~~^^^^~~^^^^^^^^^^^^^^^^^
~~~~~~~~~~~~~~~~~~~~~~~~~~!???JJY7~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~?JJ???7~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
~~~~~~~~~~~~~~~~~~~~~~~~~~!???JYJ!~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~!JJJ???!~~~~~~~~~~~~~~~~~~~~~~~~~~~~
~~~~~~~~~~~~~~~~~~~~~~~~~~!??JJJ7~~~~~~~^^^^^^^^^^^^^^^^~~~~^^^^^7JJ???7~^^^^~~~~~~~~~~~~~~~~~~~~~~~
~~~~~^^^^~~~~~~~~~~~~~~~~~7???JJ~~~~~~~^^^^^^^^^^^^^^^^^^^^^^^^^^~7JJ???~^^^^~^^^^^~~~~~~~~~~~~~~~~~
***************************************************************************************************/

#pragma once

#include <string>
#include <vector>
#include <unordered_map>
#include <optional>
#include <array>

#include "kun.h"

)cc");
}