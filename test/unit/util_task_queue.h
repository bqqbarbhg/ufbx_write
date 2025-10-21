#pragma once

template <typename ThreadContext>
struct ufbxwt_task_queue
{
	ufbxwi_task_queue tq = { };
	ThreadContext main_context;

	ufbxwt_task_queue(ufbxwi_thread_pool &tp, ufbxwi_allocator &ator, size_t task_count)
	{
		ufbxwi_task_queue_opts opts = { };
		opts.max_tasks = task_count * 2;

		opts.create_thread_ctx_fn = [](void *user) -> void* {
			return new ThreadContext();
		};
		opts.free_thread_ctx_fn = [](void *user, void *ctx) {
			delete (ThreadContext*)ctx;
		};

		ufbxwi_task_queue_init(&tq, &tp, &ator, &opts);
	}

	~ufbxwt_task_queue()
	{
		ufbxwi_task_queue_free(&tq, &main_context);
	}

	template <typename F>
	ufbxwi_task_id run(F f) {
		F *func = new F(std::move(f));

		ufbxwi_task task = { };
		task.fn = [](void *user, void *ctx) {
			F *func = (F*)user;
			(*func)(*(ThreadContext*)ctx);
			delete func;
			return true;
		};
		task.user = func;

		return ufbxwi_task_push(&tq, &task, &main_context);
	}

	void complete(ufbxwi_task_id task_id) {
		ufbxwi_task_complete(&tq, task_id, &main_context);
	}
};
