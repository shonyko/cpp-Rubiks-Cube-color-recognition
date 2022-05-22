#pragma once

//#include <vector>
#include "menu.h"
#include "lab01.h"

std::vector<function<void(std::vector<MenuItem>)>> labList {
	addLab01ToMenu,
	addLab01ToMenu,
	addLab01ToMenu
};

void addLabsToMenu(std::vector<MenuItem> menuList, std::vector<int> labs) {
	for (int id : labs) {
		if (id < 0 || id >= labList.size()) continue;
		labList[id](menuList);
	}
}
