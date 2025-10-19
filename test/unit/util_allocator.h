#pragma once

#ifdef UFBXWI_FEATURE_ALLOCATOR

struct ufbxwt_allocator
{
	ufbxwi_allocator ator = { };

	ufbxwt_allocator(ufbxwi_error &error)
	{
		ator.error = &error;

		// TODO: Proper init
		ator.max_allocs = SIZE_MAX;
		ator.max_size = SIZE_MAX / 4;
	}

	~ufbxwt_allocator()
	{
		ufbxwi_free_allocator(&ator);
	}
};

#endif
