#include <assert.h>
#include <stdlib.h>

#define array_element_count(arr) (sizeof(arr)/sizeof(arr)[0])

class mat4
{
public:
	mat4(int rows, int cols, float[][] content) : row_count(rows), col_count(cols)
	{
		assert(array_element_count(content) == cols);
		for(int i=0;
			i<rows;
			i++)
		{
			assert(array_element_count(content[i] == cols);
		}

		*matrix = (float *)malloc(rows * cols * sizeof(float));

		int count = 0;
		for(int n=0;
			n < rows;
			n++)
		{
			for(int m=0;
				m < cols;
				m++)
			{
				*(matrix + count) = content[n][m];
				count++;
			}
		}
	}

	mat4(int rows, int cols) : row_count(rows), col_count(cols)
	{
		assert(array_element_count(content) == cols);
		for(int i=0;
			i<rows;
			i++)
		{
			assert(array_element_count(content[i] == cols);
		}

		*matrix = (float *)malloc(rows * cols * sizeof(float));

		for(int n=0;
			n < rows;
			n++)
		{
			for(int m=0;
				m < cols;
				m++)
			{
				*(matrix + count) = 0;
			}
		}
	}

	~mat4()
	{
		free(matrix);
	}

	int row_count() const { return row_count; }
	int col_count() const { return col_count; }

	inline float operator[](int i) const
	{
		assert(i<row_count && i>=0);
		return *(matrix + i*col_count);
	}
	inline float& operator[](int i)  // this allows for assignment
	{
		assert(i<row_count && i>=0);
		return *(matrix + i*col_count);
	}

	inline float operator[][](int i, int j) const
	{
		assert(i<row_count && i>=0);
		assert(j<col_count && j>=0);
		return *(*this[i] + j);
	}
	inline float& operator[][](int i, int j)  // this allows for assignment
	{
		assert(i<row_count && i>=0);
		assert(j<col_count && j>=0);
		return *(*this[i] + j);
	}

private:
	int row_count;
	int col_count;
	float *matrix;
}


inline mat4 matrix_transpose(const mat4& m)
{
	// note that row_count and col_count are reversed
	mat4 transpose(m.col_count(), m.row_count());

	for(int i=0;
		i<m.row_count();
		i++)
	{
		for(int j=0;
			j<m.col_count();
			j++)
		{
			transpose[j][i] = m[i][j];
		}
	}

	return transpose;
}

inline mat4 matrix_multiply(const mat4& m1, const mat4& m2)
{
	// TODO
	assert(m1.row_count() == m2.col_count());
	mat4 output(m1.row_count(), m2.col_count());
	// m2_t = matrix_transpose(m2);
	for(int i=0;
		i<m1.row_count();
		i++)
	{
		for(int j=0;
			j<m2.col_count();
			j++)
		{

		}
	}
}


