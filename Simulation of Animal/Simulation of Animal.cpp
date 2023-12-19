#include <iostream>
#include <windows.h>
#include <random>
#include <fstream>
#include <string>
#include <vector>

using namespace std;

// Объявление переменных для сбора статистики
int preds_born = 0;
int preds_die = 0;
int preds_stopped = 0;

int herbs_born = 0;
int herbs_die = 0;
int herbs_eaten = 0;
int herbs_super_eaten = 0;

int grass_generated = 0;
int grass_destroyed = 0;
int grass_eaten = 0;

int cataclism_count = 0;

// Функция для корректного ввода чисел в заданном диапазоне
int ichek(int min, int max, string x = "") {
	int res;
	bool b, t = 0;
	if (x.length() == 0) {
		t = 1;
	}
	while (1) {
		if (t) {
			cin >> x;
		}
		b = 1;
		if ((x[0] == '-') || (x[0] >= '0' && x[0] <= '9')) {
			for (int i = 1; i < x.length(); ++i) {
				if (not((x[i] >= '0' && x[i] <= '9'))) {
					b = 0;
					break;
				}
			}
			try {
				if (b) {
					res = stoi(x);
					if (res >= min && res <= max) {
						return res;
					}
					else {
						cout << "Число не попадает в необходимый диапазон, повторите ввод: " << endl;
						cin.clear();
						cin.ignore();
						continue;
					}
				}
			}
			catch (out_of_range const& e) {
				cout << endl << "Число слишком большое" << endl;
				cin.clear();
				cin.ignore();
				continue;
			}
		}
		cout << "Неверный формат данных, введите целое число: " << endl;
		cin.clear();
		cin.ignore();
	}
}

// Создание структуры животного с необходимыми параметрами
struct entity {
	bool alive = 0;
	int age = 0;
	int hung = 0;
};

// Процедура, генерирующая матрицы с животными
void fill_with_entities(vector<vector<entity>>& fill_vect, vector<vector<entity>>& chek_vect, int need_count, int current_hungry) {
	random_device rd;
	mt19937 gen(rd());
	int size = fill_vect.size();
	int x;
	int y;
	while (need_count > 0) {
		// Генерация случайных координат
		x = gen() % size;
		y = gen() % size;
		if (!(fill_vect[y][x].alive) && !(chek_vect[y][x].alive)) {
			// Если на клетке с		данными координатами нет других животных - создать
			fill_vect[y][x].alive = 1;
			fill_vect[y][x].hung = current_hungry;
			--need_count;
		}
	}
}

// Процедура для заполнения матрицы с травой
void fill_with_grass(vector<vector<char>>& field, vector<vector<entity>>& predators, int recovery_count) {
	random_device rd;
	mt19937 gen(rd());

	int size = field.size();

	// Генерация случайного процента от общего восстановления травы для особенных клеток
	int super_grass_count = recovery_count * (gen() % 10) / 100;
	int hard_grass_count = recovery_count * (gen() % 10) / 100;

	// Объявление переменных для избежания ошибок при генерации
	int attempts = recovery_count;
	int attempts_s = super_grass_count;
	int attempts_h = hard_grass_count;

	int x;
	int y;

	// Генерация травы в случайных местах
	while (recovery_count > 0 && attempts != 0) {
		x = gen() % size;
		y = gen() % size;
		if (field[y][x] == ' ' && !(predators[y][x].alive)) {
			field[y][x] = '.';
			--recovery_count;
			++grass_generated;
		}
		else {
			--attempts;
		}
	}

	// Генерация "супер-травы" на случайных местах, восстанавливающей втрое больше сытости
	while (super_grass_count > 0 && attempts_s != 0) {
		x = gen() % size;
		y = gen() % size;
		if (!(predators[y][x].alive)) {
			field[y][x] = '*';
			--super_grass_count;
			++grass_generated;
		}
		else {
			--attempts_s;
		}
	}

	// Генерация "зарослей", наступая на которые хищники пропускают ход
	while (hard_grass_count > 0 && attempts_h != 0) {
		x = gen() % size;
		y = gen() % size;
		if (field[y][x] == ' ' && !(predators[y][x].alive)) {
			field[y][x] = '#';
			--hard_grass_count;
			++grass_generated;
		}
		else {
			--attempts_h;
		}
	}
}

// Функция для создания строки вывода всех матриц
string show(vector<vector<entity>>& predators, vector<vector<entity>>& herbivores, vector<vector<char>>& field) {
	int size = predators.size();
	string s = " " + string(size * 2 + 1, '_') + "\n";

	// Перебор всех клеток всех матриц
	for (int i = 0; i < size; ++i) {
		s += "|";
		for (int j = 0; j < size; ++j) {
			// Проверка матрицы хищников
			if (predators[i][j].alive) {
				s += " P";
			}
			// Проверка матрицы травоядных
			else if (herbivores[i][j].alive) {
				s += " H";
			}
			// Проверка матрицы травы (поля)
			else if (field[i][j] != ' ') {
				s += " " + string(1, field[i][j]);
			}
			else {
				s += "  ";
			}
		}
		s += " |\n";
	}
	s += " " + string(size * 2 + 1, '-');
	return s + "\n\n";
}

// Функция движения животных
vector<vector<entity>> moving(vector<vector<entity>>& first, vector<vector<entity>>& second, vector<vector<char>>& field, bool ispred) {
	random_device rd;
	mt19937 gen(rd());

	int size = first.size();
	vector<vector<entity>> newOne(size, vector<entity>(size, entity()));
	int x;
	int y;

	// Перебор всех клеток для поиска живых животных
	for (int i = 0; i < size; ++i) {
		for (int j = 0; j < size; ++j) {
			if (first[i][j].alive) {
				int attempts = 10;

				while (attempts != 0) {
					// Генерация случайных координат и ограничение их для предотвращения выхода за пределы поля
					x = gen() % 3 - 1;
					y = gen() % 3 - 1;
					if (i == 0) {
						y = abs(y);
					}
					if (j == 0) {
						x = abs(x);
					}
					if (i == size - 1) {
						y = -abs(y);
					}
					if (j == size - 1) {
						x = -abs(x);
					}

					// Условие проверки отсутствия другого животного на клетке, куда совершается ход
					if (!newOne[i + y][j + x].alive && !first[i + y][j + x].alive && !second[i + y][j + x].alive) {
						// Проверка попадания хищника в "заросли"
						if (ispred && field[i + y][j + x] == '#') {
							field[i + y][j + x] = ' ';
							++preds_stopped;
							break;
						}
						// Перемещение животного
						newOne[i + y][j + x] = first[i][j];
						if (ispred && field[i + y][j + x] != ' ') {
							field[i + y][j + x] = ' ';
							++grass_destroyed;
						}
						break;
					}
					--attempts;
				}
			}
		}
	}
	return newOne;
}

// Процедура для проверки шанса умереть каждого животного в зависимости от голода и возраста
void death(vector<vector<entity>>& animal, float maxAge, float needHung, bool ispred) {
	random_device rd;
	mt19937 gen(rd());
	int msize = animal.size();

	// Перебор всех клеток
	for (int i = 0; i < msize; ++i) {
		for (int j = 0; j < msize; ++j) {
			if (animal[i][j].alive) {
				// Генерация случайного шанса на смерть в зависимости от возраста относительно максимального
				int chance = (animal[i][j].age / maxAge) * 100;
				int rand = gen() % 100;
				// Смерть животного, если недостаточно сытости и сработал шанс
				if (rand <= chance && (animal[i][j].hung < needHung) || chance == 100) {
					animal[i][j] = entity();
					if (ispred) {
						++preds_die;
					}
					else
					{
						++herbs_die;
					}
				}
			}
		}
	}
}

// Процедура охоты для хищников
void hunting(vector<vector<entity>>& predators, vector<vector<entity>>& herbivores) {
	int size = predators.size();

	// Перебор всех клеток
	for (int y = 0; y < size; ++y) {
		for (int x = 0; x < size; ++x) {
			if (predators[y][x].alive) {
				vector<vector<int>> coord;

				// Создание координат для проверки клеток воруг хищника
				int y_st = y - 1, y_ed = y + 1;
				int x_st = x - 1, x_ed = x + 1;

				if (y == 0) {
					y_st = y;
				}
				if (x == 0) {
					x_st = x;
				}
				if (y == size - 1) {
					y_ed = y;
				}
				if (x == size - 1) {
					x_ed = x;
				}

				// Перебор клеток вокруг хищника
				for (int i = y_st; i <= y_ed; ++i) {
					for (int j = x_st; j <= x_ed; ++j) {
						// Если обнаружился травоядный - добавить его координаты в массив
						if (herbivores[i][j].alive) {
							coord.push_back({ i,j });
						}
					}
				}

				// Условие проверки присутствия травоядных вокруг и "съедение" случайного из найденных
				if (coord.size() != 0) {
					random_device rd;
					mt19937 gen(rd());
					int ch = gen() % coord.size();
					herbivores[coord[ch][0]][coord[ch][1]] = entity();
					++predators[y][x].hung;
					++herbs_eaten;
				}
			}
		}
	}
}

// Процедура еды для травоядных
void eating(vector<vector<entity>>& herbivores, vector<vector<char>>& field) {
	random_device rd;
	mt19937 gen(rd());

	int size = herbivores.size();

	// Перебор всех клеток для поиска траввоядных
	for (int y = 0; y < size; ++y) {
		for (int x = 0; x < size; ++x) {
			if (herbivores[y][x].alive) {
				vector<vector<int>> super_grass;
				vector<vector<int>> regular_grass;

				// Генерация координат для перебора клеток вокруг травоядного
				int y_st = y - 1, y_ed = y + 1;
				int x_st = x - 1, x_ed = x + 1;

				if (y == 0) {
					y_st = y;
				}
				if (x == 0) {
					x_st = x;
				}
				if (y == size - 1) {
					y_ed = y;
				}
				if (x == size - 1) {
					x_ed = x;
				}
				for (int i = y_st; i <= y_ed; ++i) {
					for (int j = x_st; j <= x_ed; ++j) {
						// Если найдена "супер-трава" - добавить её координаты в один массив
						if (field[i][j] == '*') {
							super_grass.push_back({ i,j });
						}
						// Если найдена обычная трава - добавить её в другой массив
						if (field[i][j] == '.') {
							regular_grass.push_back({ i,j });
						}
					}
				}
				// Если найдена хоть одна "супер-трава" - съесть её
				if (super_grass.size() != 0) {
					int ch = gen() % super_grass.size();
					field[super_grass[ch][0]][super_grass[ch][1]] = ' ';
					herbivores[y][x].hung += 3;
					++herbs_super_eaten;
				}
				// Если найдена только обычная трава - съесть случайную из них
				else if (regular_grass.size() != 0) {
					int ch = gen() % regular_grass.size();
					field[regular_grass[ch][0]][regular_grass[ch][1]] = ' ';
					++herbivores[y][x].hung;
					++grass_eaten;
				}
			}
		}
	}
}

// Процедура рождения новых животных
void birth(vector<vector<entity>>& first, vector<vector<entity>>& second, vector<vector<char>>& third, bool ispred,
	int satiety, int minAge, int maxAge, int chance) {
	random_device rd;
	mt19937 gen(rd());
	int size = first.size();
	for (int y = 0; y < size; ++y) {
		for (int x = 0; x < size; ++x) {
			if (first[y][x].alive && minAge <= first[y][x].age && first[y][x].age <= maxAge) {
				vector<vector<int>> coords;
				bool hasPara = false;
				int cnt = 0;

				// Генерация координат для перебора клеток вокруг животного
				int y_st = y - 1, y_ed = y + 1;
				int x_st = x - 1, x_ed = x + 1;

				if (y == 0) {
					y_st = y;
				}
				if (x == 0) {
					x_st = x;
				}
				if (y == size - 1) {
					y_ed = y;
				}
				if (x == size - 1) {
					x_ed = x;
				}

				// Перебор клеток вокруг животного
				for (int i = y_st; i <= y_ed; ++i) {
					for (int j = x_st; j <= x_ed; ++j) {
						// Если найдена пустая клетка - сохранить её координаты
						if (!second[i][j].alive && !first[i][j].alive) {
							coords.push_back({ i,j });
						}
						else {
							// Если найден половозелый партнёр - сохранить результат в соответствующую переменную
							if (first[i][j].alive && minAge <= first[i][j].age && first[i][j].age <= maxAge && !(y == i && x == j)) {
								hasPara = true;
								++cnt;
							}
							// Если найдены другие соседи
							else if (first[i][j].alive || second[i][j].alive) {
								++cnt;
							}
						}
					}
				}
				int rand = gen() % 100;

				// Если найден только один сосед, и он - половозрелое животное нужного вида
				if (coords.size() != 0 && hasPara && cnt == 2 && rand <= chance) {
					int ch = gen() % coords.size();
					// Создание нового животного в случайной из пустых клеток
					first[coords[ch][0]][coords[ch][1]].alive = true;
					first[coords[ch][0]][coords[ch][1]].hung = satiety;
					if (ispred) {
						third[coords[ch][0]][coords[ch][1]] = ' ';
						++preds_born;
					}
					else {
						++herbs_born;
					}
				}
			}
		}
	}
}

// Процедура увеличения возраста
void adulting(vector<vector<entity>>& animal) {
	for (int i = 0; i < animal.size(); ++i) {
		for (int j = 0; j < animal.size(); ++j) {
			if (animal[i][j].alive) {
				++animal[i][j].age;
			}
		}
	}
}

// Процедура случайного катаклизма
void cataclism(vector<vector<entity>>& predators, vector<vector<entity>>& herbivores, vector<vector<char>>& field, int isdeath) {
	random_device rd;
	mt19937 gen(rd());
	int chanse = gen() % 100;

	// Если сработал шанс на катаклизм
	if (isdeath > chanse) {
		++cataclism_count;

		// Генерация случайных координат для области катаклизма
		int part = gen() % 4;
		int size = predators.size();
		int x_st = 0;
		int y_st = 0;
		int x_ed = size;
		int y_ed = size;

		// Выбор случайной четверти поля
		if (part == 0) {
			x_ed /= 2;
			y_ed /= 2;
		}
		else if (part == 1) {
			x_st = size / 2;
			y_ed /= 2;
		}
		else if (part == 2) {
			y_st = size / 2;
			x_ed /= 2;
		}
		else {
			x_st = size / 2;
			y_st = size / 2;
		}
		for (int i = y_st; i < y_ed; ++i) {
			for (int j = x_st; j < x_ed; ++j) {

				// Удаление травы
				if (field[i][j] != ' ') {
					field[i][j] = ' ';
					++grass_destroyed;
				}
				// Смерть хищника
				if (predators[i][j].alive) {
					predators[i][j] = entity();
					++preds_die;
				}
				// Смерть травоядного
				if (herbivores[i][j].alive) {
					herbivores[i][j] = entity();
					++herbs_die;
				}

			}
		}
		
	}
}

// Функция для проверки присутствия животных на поле
bool is_anybody_there(vector<vector<entity>>& predators, vector<vector<entity>>& herbivores) {
	for (int i = 0; i < predators.size(); ++i) {
		for (int j = 0; j < herbivores.size(); ++j) {
			if (predators[i][j].alive || herbivores[i][j].alive) {
				return false;
			}
		}
	}
	return true;
}

// Процедура для уменьшения сытости животных
void setHungry(vector<vector<entity>>& animal, int needSatiety) {
	for (int i = 0; i < animal.size(); ++i) {
		for (int j = 0; j < animal.size(); ++j) {
			if (animal[i][j].alive) {
				animal[i][j].hung -= needSatiety;
			}
		}
	}
}

// Функция генерации строки со статистикой
string printStats() {
	string s = "Статистика\n";
	s += "Хищников родилось - " + to_string(preds_born) + '\n';
	s += "Хищников умерло - " + to_string(preds_die) + '\n';
	s += '\n';
	s += "Травоядных родилось - " + to_string(herbs_born) + '\n';
	s += "Травоядных умерло - " + to_string(herbs_die) + '\n';
	s += '\n';
	s += "Травы сгенерировано - " + to_string(grass_generated) + '\n';
	s += "Травы уничтожено - " + to_string(grass_destroyed) + '\n';
	s += '\n';
	s += "Травоядных съедено - " + to_string(herbs_eaten) + '\n';
	s += "Травы съедено - " + to_string(grass_eaten) + '\n';
	s += "Супер-травы съедено - " + to_string(herbs_super_eaten) + '\n';
	s += '\n';
	s += "Хищников остановлено - " + to_string(preds_stopped) + '\n';
	s += '\n';
	s += "Количество катаклизмов - " + to_string(cataclism_count) + '\n';

	return s;
}

// Структура для параметров по-умолчанию
struct parametrs {
	int field_size = 30;
	int sim_duration = 30;

	int predator_count = 50;
	int predator_age = 10;
	int predator_start = 2;
	int predator_end = 7;
	int predator_born_chanse = 70;
	int predator_satiety = 4;

	int herbivores_count = 100;
	int herbivores_age = 15;
	int herbivores_start = 3;
	int herbivores_end = 10;
	int herbivores_born_chanse = 90;
	int herbivores_satiety = 6;

	int grass_count = 450;

	int cataclism_event_chanse = 2;

	int start_season = 0;
};

// Функция генерации строки с начальными параметрами
string start_parms(parametrs p, string seasons[]) {
	string s = "";
	s += "Размер поля: " + to_string(p.field_size * p.field_size) + '\n';
	s += "Длительность симуляции: " + to_string(p.sim_duration) + '\n';
	s += '\n';
	s += "Хищники:\n";
	s += "Изначальное количество: " + to_string(p.predator_count) + '\n';
	s += "Максимальный возраст: " + to_string(p.predator_age) + '\n';
	s += "Начало репродуктивного возраста: " + to_string(p.predator_start) + '\n';
	s += "Конец репродуктивного возраста: " + to_string(p.predator_end) + '\n';
	s += "Шанс на рождение: " + to_string(p.predator_born_chanse) + '\n';
	s += "Необходимое количество еды: " + to_string(p.predator_satiety) + '\n';
	s += '\n';
	s += "Травоядные:\n";
	s += "Изначальное количество: " + to_string(p.herbivores_count) + '\n';
	s += "Максимальный возраст: " + to_string(p.herbivores_age) + '\n';
	s += "Начало репродуктивного возраста: " + to_string(p.herbivores_start) + '\n';
	s += "Конец репродуктивного возраста: " + to_string(p.herbivores_end) + '\n';
	s += "Шанс на рождение: " + to_string(p.herbivores_born_chanse) + '\n';
	s += "Необходимое количество еды: " + to_string(p.herbivores_satiety) + '\n';
	s += '\n';
	s += "Изначальное количество травы: " + to_string(p.grass_count) + '\n';
	s += "Шанс на катаклизм: " + to_string(p.cataclism_event_chanse) + '\n';
	s += "Начальное время года: " + seasons[p.start_season] + '\n';

	return s;
}


// Основная функция симуляции
void animals(parametrs p) {
	random_device rd;
	mt19937 gen(rd());


	int size = p.field_size;
	string seasons[4] = { "Весна", "Лето", "Осень", "Зима" };


	vector<vector<entity>> predators(size, vector<entity>(size, entity()));
	vector<vector<entity>> herbivores(size, vector<entity>(size, entity()));

	vector<vector<char>> field(size, vector<char>(size, ' '));

	// Заполнение матриц с хищниками и травоядными
	fill_with_entities(predators, herbivores, p.predator_count, p.predator_satiety / 2);
	fill_with_entities(herbivores, predators, p.herbivores_count, p.herbivores_satiety / 2);

	// Заполнение матрицы с травой
	fill_with_grass(field, predators, p.grass_count);

	ofstream logs("logs.txt");

	// Вывод начальных параметров и условий в консоль и в файл
	string output_text = "";
	string output_field = show(predators, herbivores, field);
	output_text += "Установленные параметры:\n";
	output_text += start_parms(p, seasons);
	output_text += "Начальные условия:\n";
	output_text += output_field;


	logs << output_text << '\n';


	system("cls");
	fwrite(output_text.c_str(), 1, output_text.length(), stdout);
	system("pause");
	system("cls");


	int current_season = p.start_season;
	bool isExit = false;

	// Симуляция лет и месяцев друг за другом
	for (int year = 1; year <= p.sim_duration; ++year) {
		for (int month = 1; month <= 12; ++month) {

			if ((month - 1) % 3 == 0) {
				++current_season;
				if (current_season == 4) {
					current_season = 0;
				}
			}
			
			// Функция катаклизма
			cataclism(predators, herbivores, field, p.cataclism_event_chanse);

			// Смерть голодных животных
			death(predators, p.predator_age, p.predator_satiety, true);
			death(herbivores, p.herbivores_age, p.herbivores_satiety, false);

			int grass_recovery_count = (p.grass_count / 5);

			// Изменение количества восстанавливаемой травы в соответствии с временем года
			switch (current_season) {
			case 0: // Весна
				grass_recovery_count /= 2;
				break;
			case 1: // Лето
				++grass_recovery_count;
				break;
			case 2: // Осень
				grass_recovery_count /= 2;
				break;
			case 3: // Зима
				grass_recovery_count = 0;
				break;
			}
			// Генерация новой травы
			fill_with_grass(field, predators, (grass_recovery_count));

			// Охота хищников и еда травоядных
			hunting(predators, herbivores);
			eating(herbivores, field);

			// Рождение новых животных
			birth(predators, herbivores, field, true, p.predator_satiety / 2, p.predator_start, p.predator_end, p.predator_born_chanse);
			birth(herbivores, predators, field, false, p.herbivores_satiety / 2, p.herbivores_start, p.herbivores_end, p.herbivores_born_chanse);

			// Вывод в консоль и в файл
			output_text = "Год: " + to_string(year) + ", Месяц: " + to_string(month) + ", Время года: " + seasons[current_season] + '\n';
			output_field = show(predators, herbivores, field);

			logs << output_text + output_field << '\n';

			output_text += output_field + '\n';
			system("cls");
			fwrite(output_text.c_str(), 1, output_text.length(), stdout);
			Sleep(200);

			// Передвижение животных по полю
			predators = moving(predators, herbivores, field, true);
			herbivores = moving(herbivores, predators, field, false);

			output_text = "Год: " + to_string(year) + ", Месяц: " + to_string(month) + ", Время года: " + seasons[current_season] + '\n';
			output_field = show(predators, herbivores, field);

			logs << output_text + output_field << '\n';

			output_text += output_field;
			system("cls");
			fwrite(output_text.c_str(), 1, output_text.length(), stdout);
			Sleep(200);

			// Завершение симуляции если животных не осталось
			if (is_anybody_there(predators, herbivores)) {
				isExit = true;
				break;
			}
		}
		if (isExit) {
			break;
		}
		// Установка голода раз в год
		setHungry(predators, p.predator_satiety / 2);
		setHungry(herbivores, p.herbivores_satiety / 2);
		// Прибавление возраста раз в год
		adulting(predators);
		adulting(herbivores);
	}

	if (isExit) {
		output_text = "Все популяции существ вымерли, конец симуляции\n";
	}
	else {
		output_text = "Симуляция завершена\n";
	}

	output_text += printStats();

	logs << output_text;

	// Вывод статистики
	system("cls");
	fwrite(output_text.c_str(), 1, output_text.length(), stdout);
	system("pause");
}


int main() {
	setlocale(LC_ALL, "Russian");

	// Выбор пользователем режима - параметры вручную или по умолчанию
	cout << "Введите режим определение начальных параметров (0 - автоматические параметры, 1 - ручная установка): ";
	int mode = ichek(0, 1);
	if (mode == 0) {
		animals(parametrs());
	}
	else {
		parametrs parms;

		// Ввод пользователем параметров с защитой
		int m = 1000;
		cout << "Введите ширину поля: ";
		int size = ichek(1, m);
		parms.field_size = size;
		cout << "Введите количество лет симуляции: ";
		parms.sim_duration = ichek(1, m);

		cout << "Введите начальное количество хищников: ";
		parms.predator_count = ichek(0, size * size);
		cout << "Введите максимальный возраст хищников: ";
		parms.predator_age = ichek(1, m);
		cout << "Введите минимальный репродуктивный возраст хищников: ";
		parms.predator_start = ichek(0, m);
		cout << "Введите максимальный репродуктивный возраст хищников: ";
		parms.predator_end = ichek(parms.predator_start, m);
		cout << "Введите шанс рождаемости хищников (от 0% до 100%): ";
		parms.predator_born_chanse = ichek(0, 100);
		cout << "Введите количество травоядных, необходимых хищнику: ";
		parms.predator_satiety = ichek(0, m);

		cout << "Введите начальное количество травоядных: ";
		parms.herbivores_count = ichek(0, size * size - parms.predator_count);
		cout << "Введите максимальный возраст травоядных: ";
		parms.herbivores_age = ichek(1, m);
		cout << "Введите минимальный репродуктивный возраст травоядных: ";
		parms.herbivores_start = ichek(0, m);
		cout << "Введите максимальный репродуктивный возраст травоядных: ";
		parms.herbivores_end = ichek(parms.herbivores_start, m);
		cout << "Введите шанс рождаемость травоядных (от 0% до 100%): ";
		parms.herbivores_born_chanse = ichek(0, 100);
		cout << "Введите количество травы, необходимой травоядному: ";
		parms.herbivores_satiety = ichek(0, m);

		cout << "Введите изначальное количество травы: ";
		parms.grass_count = ichek(0, size * size);
		cout << "Введите шанс на природный катаклизм в месяц (от 0 до 100%): ";
		parms.cataclism_event_chanse = ichek(0, 100);
		cout << "Введите начальное время года (весна - 0, лето - 1, осень - 2, зима - 3): ";
		parms.start_season = ichek(0, 3);
		animals(parms);
	}
	return 0;
}