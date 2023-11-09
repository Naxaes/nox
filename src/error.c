#include "error.h"

#define SPACES "                                                                        "
#define DASHES "------------------------------------------------------------------------"
#define ARROWS "^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^"


void point_to_error(Str source, int start, int end) {
    Location loc_start = location_of(source.data, start);
    Location loc_end   = location_of(source.data, end);

    if (loc_start.row == loc_end.row) {
        Str line_start = str_line_at(source, start);

        int line_number_width = 0;
        int row = loc_start.row;
        while (row > 0) {
            line_number_width += 1;
            row /= 10;
        }

        fprintf(stderr, " %d |     ", loc_start.row);
        fprintf(stderr, STR_FMT "\n", STR_ARG(line_start));

        fprintf(stderr, " " STR_FMT " | " , line_number_width, SPACES);
        fprintf(stderr, STR_FMT STR_FMT "\n", 4+loc_start.column-1, DASHES, (end-start), ARROWS);
    } else {
        Str line_start = str_line_at(source, start);
        Str line_end   = str_line_at(source, end);

        int line_number_width_end = 0;
        int row_end = loc_end.row;
        while (row_end > 0) {
            line_number_width_end += 1;
            row_end /= 10;
        }

        fprintf(stderr, " %.*d |     ", line_number_width_end, loc_start.row);
        fprintf(stderr, STR_FMT "\n", STR_ARG(line_start));

        fprintf(stderr, " %.*s | " , line_number_width_end, SPACES);
        fprintf(stderr, STR_FMT STR_FMT "\n", 4, DASHES, (int)line_start.size, ARROWS);

        for (int i = loc_start.row+1; i < loc_end.row; ++i) {
            fprintf(stderr, " %.*d | |   \n", line_number_width_end, i);
        }

        fprintf(stderr, " %.*d | |   ", line_number_width_end, loc_end.row);
        fprintf(stderr, STR_FMT "\n", STR_ARG(line_end));

        fprintf(stderr, " %.*s | " , line_number_width_end, SPACES);
        fprintf(stderr, STR_FMT STR_FMT "\n", 4, DASHES, (int)line_end.size, ARROWS);
    }
}
