#include "error.h"
#include "logger.h"

#define SPACES "                                                                        "
#define DASHES "------------------------------------------------------------------------"
#define ARROWS "^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^"


void point_to_error(Logger* logger, Str source, int start, int end) {
    Location loc_start = location_of(source.data, start);
    Location loc_end   = location_of(source.data, end);
        
    int line_number_width = 0;
    int row = loc_end.row;
    while (row > 0) {
        line_number_width += 1;
        row /= 10;
    }

    if (loc_start.row == loc_end.row) {
        Str line_start = str_line_at(source, loc_start.index);
        
        logger_extend(logger, LOG_LEVEL_ERROR, " %d |     ", loc_start.row);
        logger_extend(logger, LOG_LEVEL_ERROR, STR_FMT "\n", STR_ARG(line_start));

        logger_extend(logger, LOG_LEVEL_ERROR, " " STR_FMT " | " , line_number_width, SPACES);
        logger_extend(logger, LOG_LEVEL_ERROR, STR_FMT STR_FMT "\n", 4+loc_start.column-1, DASHES, (loc_end.column-loc_start.column), ARROWS);
    } else {
        Str line_start = str_line_at(source, loc_start.index);
        Str line_end   = str_line_at(source, loc_end.index);
        
        logger_extend(logger, LOG_LEVEL_ERROR, " %.*d |     ", line_number_width, loc_start.row);
        logger_extend(logger, LOG_LEVEL_ERROR, STR_FMT "\n", STR_ARG(line_start));

        logger_extend(logger, LOG_LEVEL_ERROR, " %.*s | " , line_number_width, SPACES);
        logger_extend(logger, LOG_LEVEL_ERROR, STR_FMT STR_FMT "\n", 4, DASHES, (int)line_start.size, ARROWS);

        for (int i = loc_start.row+1; i < loc_end.row; ++i) {
            logger_extend(logger, LOG_LEVEL_ERROR, " %.*d | |   \n", line_number_width, i);
        }

        logger_extend(logger, LOG_LEVEL_ERROR, " %.*d | |   ", line_number_width, loc_end.row);
        logger_extend(logger, LOG_LEVEL_ERROR, STR_FMT "\n", STR_ARG(line_end));

        logger_extend(logger, LOG_LEVEL_ERROR, " %.*s | " , line_number_width, SPACES);
        logger_extend(logger, LOG_LEVEL_ERROR, STR_FMT STR_FMT "\n", 4, DASHES, (int)line_end.size, ARROWS);
    }
}
