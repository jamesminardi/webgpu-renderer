#pragma once

#include <iostream>
#include <vector>
#include <array>
#include <iomanip>


/**
 * Grid class that represents a 2D grid of elements of type T
 *
 * @tparam T type of elements in the grid
 * @tparam N size of the grid (N x N)
 */
template <typename T, uint16_t N>
class Grid {

private:
	std::array<T, N*N> arr;
	const int size = N; // Number of rows and columns

public:

	// Copy constructor from grid
	Grid(const Grid<T,N>& other) : arr(other.arr) { }

	// Copy constructor from array
	Grid(const std::array<T,N*N>& arr) : arr(arr) { }

	// Move constructor from array
	Grid(std::array<T,N*N>&& arr) : arr(std::move(arr)) { }

	// Move constructor from grid
	Grid(Grid<T,N>&& other) : arr(std::move(other.arr)) { }

	// Default constructor
	Grid(T t = T{}) {
		arr.fill(t);
	}

	// Default Destructor
	~Grid() = default;

	// Copy assignment operator
	Grid<T,N>& operator=(const Grid<T,N>& rhs) {
		arr = rhs.arr;
		return *this;
	}

	// Move assignment operator
	Grid<T,N>& operator=(Grid<T,N>&& rhs) {
		arr = std::move(rhs.arr);
		return *this;
	}

	T& operator()(int row, int col) {
		return arr[row * size + col];
	}

	T operator()(int row, int col) const {
		return arr[row * size + col];
	}

	T& operator()(int idx) {
		return arr[idx];
	}

	T operator()(int idx) const {
		return arr[idx];
	}

	int getRow(int idx) const {
		return idx / size;
	}

	int getCol(int idx) const {
		return idx % size;
	}

	int getIndex(int row, int col) const {
		return row * size + col;
	}

	int getIndex(int idx) const {
		return idx;
	}

	int getSize() const {
		return size;
	}

	int getArea() const {
		return size * size;
	}

	std::array<T,N*N>& getArray() {
		return arr;
	}

	const std::array<T,N*N>& getArray() const {
		return arr;
	}


	/**
	 * Generates numbered indices in a grid with negative indices indicating the border.
	 *
	 * Ex.
	 *	 -12 -13 -14 -15  -16
	 *	 -10   6   7   8  -11
	 *	  -8   3   4   5  -9
	 *	  -6   0   1   2  -7
	 *	  -1  -2  -3  -4  -5
	 *
	 * @param  borderWidth 	Width of the border
	 * @return 				Grid of indices
	 */
	static Grid<int, N> generateBorderedGrid(const int borderWidth /*Width of the border*/) {
		Grid<int, N> grid;
		int meshIdx = 0;
		int borderIdx = -1;

		for (int row = 0; row < N; row++) {
			for (int col = 0; col < N; col++) {
				bool isBorder = col < borderWidth || col >= N - borderWidth || row < borderWidth || row >= N - borderWidth;
				if (isBorder) {
					grid(row, col) = borderIdx;
					borderIdx--;
				}
				else {
					grid(row, col) = meshIdx;
					meshIdx++;
				}
			}
		}

		// Print from top to bottom
//		std::cout << "Bordered Grid:" << std::endl;
//		for (int row = N-1; row >=0; row--) {
//			for (int col = 0; col < N; col++) {
//				std::cout << std::setw(4) << std::right << grid(row, col) << " ";
//			}
//			std::cout << std::endl;
//		}

		return grid;
	}


};
