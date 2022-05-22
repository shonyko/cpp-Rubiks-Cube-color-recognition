#pragma once

#ifndef MENU_H
#define MENU_H

#include <utility>
#include <string>
#include <functional>
#include <vector>

typedef std::pair<std::string, std::function<void()>> MenuItem;

int getNoCharacters(int nb);

MenuItem createMenuItem(std::string name, std::function<void()> func);

void printMenu(std::vector<MenuItem> menuList);

void handleOption(int op, std::vector<MenuItem> menuList);

#endif
