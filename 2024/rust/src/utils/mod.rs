use std::fmt::Debug;
use std::ops::Index;
use std::ops::IndexMut;

#[derive(Clone, Copy)]
pub struct MatrixView<'a, T> {
    pub data: &'a [T],
    pub extents: (usize, usize),
}

impl<'a, T> MatrixView<'a, T> {
    pub fn new(data: &'a [T], extents: (usize, usize)) -> Self {
        Self { data, extents }
    }
}

pub trait Matrix: Index<(usize, usize)> {
    type Item;
    fn extents(&self) -> (usize, usize);
    fn in_bounds(&self, (i, j): (usize, usize)) -> bool {
        i < self.extents().0 && j < self.extents().1
    }
    fn get(&self, indices: (usize, usize)) -> Option<&Self::Item>;
}

pub trait MatrixMut: Matrix + IndexMut<(usize, usize)> {
    fn get_mut(&mut self, indices: (usize, usize)) -> Option<&mut Self::Item>;
}

impl<T> MatrixView<'_, T> {
    fn extents(&self) -> (usize, usize) {
        self.extents
    }
}

impl<T> Index<(usize, usize)> for MatrixView<'_, T> {
    type Output = T;

    fn index(&self, index: (usize, usize)) -> &Self::Output {
        &self.data[self.extents().1 * index.0 + index.1]
    }
}

pub struct MatrixViewMut<'a, T> {
    extents: (usize, usize),
    data: &'a mut [T],
}

impl<'a, T> Index<(usize, usize)> for MatrixViewMut<'a, T> {
    type Output = T;

    fn index(&self, index: (usize, usize)) -> &Self::Output {
        &self.data[self.extents.1 * index.0 + index.1]
    }
}

impl<T> Matrix for MatrixView<'_, T> {
    type Item = T;

    fn extents(&self) -> (usize, usize) {
        self.extents
    }

    fn get(&self, indices: (usize, usize)) -> Option<&Self::Item> {
        if self.in_bounds(indices) {
            Some(&self[indices])
        } else {
            None
        }
    }
}

impl<T> IndexMut<(usize, usize)> for MatrixViewMut<'_, T> {
    fn index_mut(&mut self, index: (usize, usize)) -> &mut Self::Output {
        &mut self.data[self.extents.1 * index.0 + index.1]
    }
}

impl<T> Matrix for MatrixViewMut<'_, T> {
    type Item = T;

    fn extents(&self) -> (usize, usize) {
        self.extents
    }

    fn get(&self, indices: (usize, usize)) -> Option<&Self::Item> {
        if self.in_bounds(indices) {
            Some(&self[indices])
        } else {
            None
        }
    }
}

impl<T> MatrixMut for MatrixViewMut<'_, T> {
    fn get_mut(&mut self, indices: (usize, usize)) -> Option<&mut Self::Item> {
        if self.in_bounds(indices) {
            Some(&mut self[indices])
        } else {
            None
        }
    }
}

pub struct MatrixBuf<T> {
    vec: Vec<T>,
    extents: (usize, usize),
}

impl<T> MatrixBuf<T> {
    pub fn new(vec: Vec<T>, extents: (usize, usize)) -> Self {
        Self { vec, extents }
    }
    pub fn spread(t: T, extents: (usize, usize)) -> Self
    where
        T: Clone,
    {
        Self {
            vec: vec![t; extents.0 * extents.1],
            extents: (extents.0, extents.1),
        }
    }

    pub fn as_view(&self) -> MatrixView<T> {
        MatrixView {
            data: &self.vec,
            extents: self.extents,
        }
    }
    pub fn as_view_mut(&mut self) -> MatrixViewMut<T> {
        MatrixViewMut {
            data: &mut self.vec,
            extents: self.extents,
        }
    }
}

impl<T> From<MatrixBuf<T>> for Vec<T> {
    fn from(value: MatrixBuf<T>) -> Self {
        value.vec
    }
}

impl<T: Debug> Debug for MatrixView<'_, T> {
    fn fmt(&self, f: &mut std::fmt::Formatter<'_>) -> std::fmt::Result {
        write!(
            f,
            "{:?}",
            self.data.chunks(self.extents.1).collect::<Vec<_>>()
        )
    }
}
