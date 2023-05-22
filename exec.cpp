/*******************************************************************************
* WiSpy                                                                        *
* Author: romtec123                                                            *
* Date: 05/22/2023                                                             *
* File: exec.cpp                                                               *
*                                                                              *
* Description: This file contains the implementation for the execute function. *
*******************************************************************************/

#include <cstdio>
#include <string>


std::string execute (const std::string & cmd)
{
    FILE* pipe = popen(cmd.c_str(), "r");

    if (!pipe)
        return "ERROR";

    char buffer[128];
    std::string result = "";

    while (fgets(buffer, 128, pipe) != NULL)
        result += buffer;

    pclose(pipe);
    return result;
}