// #include <iostream>
// #include "Admin.h"
// #include "User.h"

// using namespace std;

// int main()
// {
//     Admin admin;
//     User user;

//     int choice;

//     while (true)
//     {
//         cout << "\n=====================================\n";
//         cout << "   UNIVERSITY BUS TRACKER SYSTEM\n";
//         cout << "=====================================\n";
//         cout << "1. Admin Login\n";
//         cout << "2. User Registration\n";
//         cout << "3. User Login\n";
//         cout << "4. Exit\n";

//         cout << "\nEnter Choice : ";
//         cin >> choice;

//         switch (choice)
//         {
//         case 1:
//             if (admin.login())
//             {
//                 admin.dashboard();
//             }
//             break;

//         case 2:
//             user.registerUser();
//             break;

//         case 3:
//             if (user.loginUser())
//             {
//                 user.dashboard();
//             }
//             break;
//         case 4:
//             cout << "\nThank You!\n";
//             return 0;

//         default:
//             cout << "\nInvalid Choice!\n";
//         }
//     }

//     return 0;
// }
#include "GUI.h"

int main()
{
    GUI gui;
    gui.run();

    return 0;
}