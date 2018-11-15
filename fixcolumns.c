#include "fixcolumns.h"

#define NUMHEADERS 28

property h_headers[HMAP];

extern char* g_headers[] = {
        "color",
        "director_name",
        "num_critic_for_reviews",
        "duration",
        "director_facebook_likes",
        "actor_3_facebook_likes",
        "actor_2_name",
        "actor_1_facebook_likes",
        "gross",
        "genres",
        "actor_1_name",
        "movie_title",
        "num_voted_users",
        "cast_total_facebook_likes",
        "actor_3_name",
        "facenumber_in_poster",
        "plot_keywords",
        "movie_imdb_link",
        "num_user_for_reviews",
        "language",
        "country",
        "content_rating",
        "budget",
        "title_year",
        "actor_2_facebook_likes",
        "imdb_score",
        "aspect_ratio",
        "movie_facebook_likes"
};

extern char h_types[] = {
    STR,
    STR,
    FLOAT,
    FLOAT,
    FLOAT,
    FLOAT,
    STR,
    FLOAT,
    FLOAT,
    STR,
    STR,
    STR,
    FLOAT,
    FLOAT,
    STR,
    FLOAT,
    STR,
    STR,
    FLOAT,
    STR,
    STR,
    STR,
    FLOAT,
    FLOAT,
    FLOAT,
    FLOAT,
    FLOAT,
    FLOAT
};

char get_type(char* val) {
	property* p = get_header_p(val);
	if(p->index -1 && strcmp(val, g_headers[p->index]) == 0) return h_types[p->index];
	return 'E';
}

void print_row(datarow* row, FILE* stream) {
	int i = 0;
	for(i = 0; i < row->size; i++) {
		fprintf(stream, "%s", row->cells[i].original);
		if(i < row->size - 1) fprintf(stream, ",");
	}

	fprintf(stream, "\n");
}

void print_header(char** vals, int n, FILE* stream) {
	int i = 0;
	for(i = 0; i < n; i++) {
		fprintf(stream, "%s", vals[i]);
		if(i < n - 1) fprintf(stream, ",");
	}
	fprintf(stream, "\n");
}

datarow create_datarow(cell* cells, int col_count) {
	datarow row;
	row.size = col_count;
	row.cells = cells;
	return row;
}

table* create_table() {
	table* tab = (table*)malloc(sizeof(table));
	tab->rows = (datarow*)malloc(TABLE_SIZE * sizeof(datarow));
	tab->max_size = TABLE_SIZE;
	tab->size = 0;
	return tab;
}

void append(table* tab, datarow* row) {
	if(tab->size < tab->max_size) {
		tab->rows[tab->size] = *row;
		tab->size++;
	}
	else {
		datarow* temp = tab->rows;
		tab->max_size *= 2;
		tab->rows = (datarow*)malloc(tab->max_size * sizeof(datarow));
		int i = 0;
		for(i = 0; i < tab->size; i++)
			tab->rows[i] = temp[i];
		free(temp);
		append(tab, row);
	}
}

char** split_by_comma(char* line, int* len) {
	int i = 0, prev_i = 0;
	int count = 1;
	int max_size = 1;
	bool inside_quote = false;
	int line_length = 0;
	while(line[i] != '\0' && line[i] != '\n') {
		if(line[i] == '"')
			inside_quote = inside_quote ? false : true;
		if(line[i] == ',' && inside_quote == false) {
			count++;
			max_size = max(max_size, i - prev_i);
			prev_i = i;
		}
		i++;
		line_length++;
	}
	inside_quote = false;
	char** words = (char**)malloc(count * sizeof(char*));
	for(i = 0; i < count; i++) {
		words[i] = (char*)malloc(max_size * sizeof(char));
		words[i][0] = '\0';
	}
	int curr_word = 0;
	int curr_i = 0;
	for(i = 0; i < line_length; i++) {
		if(line[i] == '"')
        	       	inside_quote = inside_quote ? false : true;
		if(line[i] == ',' && inside_quote == false) {
			words[curr_word++][curr_i] = '\0';
			curr_i = 0;
		}
		else {
			words[curr_word][curr_i++] = line[i];
		}
	}
	*len = count;
	return words;
}


cell* get_cells(char** pre_cell, char data_type, int index, int len, char** headers) {
	int k = 0, i = 0;
	cell* cells = (cell*)malloc(NUMHEADERS * sizeof(cell));
    for(k = 0 ; k < NUMHEADERS; k++) {
        cells[k].is_empty = true;
    }
	for(k = 0; k < len; k++) {
        i = get_header_p(headers[k])->index;
		cells[i].original = pre_cell[i];
		cells[i].is_empty = pre_cell[i][0] == '\0' ? true : false;
		if(cells[i].is_empty) {
			cells[i].str_len = 0;
		}
		if(i == index) {
			cells[i].data_type = data_type;
			int st = 0;
			int end = 0;
			while(pre_cell[i][st] == ' ' || pre_cell[i][st] == '\t' || pre_cell[i][st] == '"') st++;
			while(pre_cell[i][end] != '\0') end++;
			end--;
			while(end > -1 && (pre_cell[i][end] == ' ' || pre_cell[i][end] == '\t' || pre_cell[i][end] == '"')) end--;
			if(end <= st) cells[i].is_empty = true;
			else {
				cells[i].is_empty = false;
				int j = 0;
				cells[i].str_datum = (char*)malloc((end - st + 2) * sizeof(char));
				for(j = st; j <= end; j++)
					cells[i].str_datum[j - st] = pre_cell[i][j];
				cells[i].str_datum[j - st] = '\0';
				cells[i].str_len = end - st + 1;
				if(data_type == INT)
					cells[i].num_datum = atoi(cells[i].str_datum);
				else if(data_type == FLOAT)
					cells[i].f_datum = atof(cells[i].str_datum);
			}
		}
	}
	return cells;
}

table* sort_file(char* file_path, int cell_index) {
	FILE* fp = fopen(file_path, "r");
	char sort_type = h_types[cell_index];
	char buff[BUFSIZ];
	char *read = fgets(buff, sizeof buff, fp);
	int no_of_cols = 0;
	char** headers = split_by_comma(buff, &no_of_cols);
	int i;
    for(i = 0; i < no_of_cols; i++) {
        if(exists(headers[i]) == false) {
            fprintf(stderr, "Could not sort file %s: incorrect format", file_path);
			exit(0);
        }
    }
    table* main_table = create_table();
    main_table->header = g_headers;
    read = fgets(buff, sizeof buff, fp);
    while(read != NULL) {
        int nc = 0;
        char** split_line = split_by_comma(buff, &nc);
        if(nc != no_of_cols) {
            fprintf(stderr, "Could not sort file %s: incorrect format", file_path);
            exit(0);
        }
        cell* cells = get_cells(split_line, sort_type, cell_index, nc, headers);
        datarow row = create_datarow(cells, nc);
        append(main_table, &row); 
        read = fgets(buff, sizeof buff, fp);
    }
    datarow* sorted = mergesort(main_table->rows, cell_index, main_table->size);
	main_table->rows = sorted;

	fclose(fp);
	return main_table;
}

unsigned long hash(unsigned char *str)
{
    unsigned long hash = 5381;
    int c;

    while ((c = *str++))
        hash = ((hash << 5) + hash) + c; /* hash * 33 + c */

    return hash;
}

void initialize_headers() {
	int i;
	for(i = 0; i < HMAP; i++) {
		h_headers[i].index = -1;
	}
    for(i = 0; i < NUMHEADERS; i++) {
        int hsh = hash(g_headers[i]);
        int index = hmap(hsh);
        h_headers[index].hash = hsh;
        h_headers[index].index = i;
    }
}

bool exists(char* header) {
	int hsh = hash(header);
	int index = hmap(hsh);
	if(h_headers[index].index != -1 && h_headers[index].hash == hsh && strcmp(header, g_headers[h_headers[index].index]) == 0) return true;
	return false;
}

int get_index_nc(char* header) {
	int hsh = hash(header);
    int index = hmap(hsh);
	return h_headers[index].index;
}

property* get_header_p(char* header) {
	int hsh = hash(header);
    int index = hmap(hsh);
    return &h_headers[index];
}

void testcols() {
}
