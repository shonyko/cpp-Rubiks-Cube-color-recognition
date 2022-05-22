#include "stdafx.h"
#include "menu.h"
#include <iostream>

int getNoCharacters(int nb) {
	int cnt = 0;
	while (nb != 0) {
		cnt++;
		nb /= 10;
	}

	return cnt;
}

MenuItem createMenuItem(std::string name, std::function<void()> func) {
	return make_pair(name, func);
}

void printMenu(std::vector<MenuItem> menuList) {
	int maxNumber = menuList.size() - 1;
	int noCharacters = getNoCharacters(maxNumber);

	printf("Menu:\n");
	for (int i = 0; i <= maxNumber; i++) {
		printf("%*d - ", noCharacters, i + 1);
		std::cout << menuList[i].first << std::endl;
	}
	printf("\n%*d - Exit\n\n", noCharacters, 0);
}

void handleOption(int op, std::vector<MenuItem> menuList) {
	if (op <= 0 || op > menuList.size()) return;
	op--;
	menuList[op].second();
}