#include "data-convert.h"

#include <fcntl.h>
#include <sys/stat.h>
#include <unistd.h>

#include "linux/compiler.h"
#include "linux/err.h"
#include "util/auxtrace.h"
#include "util/debug.h"
#include "util/dso.h"
#include "util/event.h"
#include "util/evsel.h"
#include "util/evlist.h"
#include "util/header.h"
#include "util/map.h"
#include "util/session.h"
#include "util/symbol.h"
#include "util/thread.h"
#include "util/tool.h"

int parse_samples(const char *input_name, const char *output_name) {
	struct perf_session *session;
	int fd;
	FILE *out;
	int ret = -1;

	struct perf_tool tool = {
		.auxtrace		= perf_event__process_auxtrace,
		.auxtrace_info		= perf_event__process_auxtrace_info,
	};

	struct perf_data data = {
		.mode = PERF_DATA_MODE_READ,
		.path = input_name,
	};

	fd = open(output_name, O_CREAT | O_WRONLY | O_EXCL, 0666);
	if (fd == -1) {
		pr_err("Error opening output file!\n");
		goto err;
	}

	out = fdopen(fd, "w");
	if (!out) {
		fprintf(stderr, "Error opening output file!\n");
		close(fd);
		goto err;
	}

	dump_trace = true;
	session = perf_session__new(&data, &tool);
	if (IS_ERR(session)) {
		fprintf(stderr, "Error creating perf session!\n");
		goto err_fclose;
	}

	perf_session__process_events(session);

	ret = 0;

	perf_session__delete(session);
err_fclose:
	fclose(out);
err:
	return ret;
}
