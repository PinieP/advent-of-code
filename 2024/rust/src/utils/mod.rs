use std::fmt::Debug;
use std::ops::Index;
use std::ops::IndexMut;

pub struct Indices {
    extents: (usize, usize),
    index: usize,
}

pub struct IndicesInner {
    index0: usize,
    index: usize,
    extent: usize,
}

impl Iterator for IndicesInner {
    type Item = (usize, usize);

    fn next(&mut self) -> Option<Self::Item> {
        if self.index < self.extent {
            let index = self.index;
            self.index += 1;
            Some((self.index0, index))
        } else {
            None
        }
    }
}

impl Iterator for Indices {
    type Item = IndicesInner;

    fn next(&mut self) -> Option<Self::Item> {
        if self.index < self.extents.0 {
            let index = self.index;
            self.index += 1;
            Some(IndicesInner {
                index0: index,
                index: 0,
                extent: self.extents.1,
            })
        } else {
            None
        }
    }
}

pub trait Matrix: Index<(usize, usize)> {
    type Item;
    fn extents(&self) -> (usize, usize);
    fn in_bounds(&self, (i, j): (usize, usize)) -> bool {
        i < self.extents().0 && j < self.extents().1
    }

    fn indices(&self) -> Indices {
        Indices {
            extents: self.extents(),
            index: 0,
        }
    }
    fn get(&self, indices: (usize, usize)) -> Option<&Self::Item>;
}

pub trait MatrixMut: Matrix + IndexMut<(usize, usize)> {
    fn get_mut(&mut self, indices: (usize, usize)) -> Option<&mut Self::Item>;
}

pub trait MatrixLayout: Clone {
    fn map_indices(&self, extents: (usize, usize), indices: (usize, usize)) -> usize;
}

#[derive(Default, Clone, Copy)]
pub struct LayoutRight {}
#[derive(Default, Clone, Copy)]
pub struct LayoutLeft {}
#[derive(Clone, Copy)]
pub struct LayoutStrided(pub usize, pub usize);

impl MatrixLayout for LayoutRight {
    fn map_indices(&self, extents: (usize, usize), indices: (usize, usize)) -> usize {
        extents.1 * indices.0 + indices.1
    }
}
impl MatrixLayout for LayoutLeft {
    fn map_indices(&self, extents: (usize, usize), indices: (usize, usize)) -> usize {
        extents.0 * indices.1 + indices.0
    }
}
impl MatrixLayout for LayoutStrided {
    fn map_indices(&self, _extents: (usize, usize), indices: (usize, usize)) -> usize {
        self.0 * indices.1 + indices.0 * self.1
    }
}

#[derive(Clone, Copy)]
pub struct MatrixView<'a, T, Layout: MatrixLayout = LayoutRight> {
    pub data: &'a [T],
    extents: (usize, usize),
    pub layout: Layout,
}

impl<'a, T, Layout: MatrixLayout> MatrixView<'a, T, Layout> {
    pub fn new(data: &'a [T], extents: (usize, usize)) -> Self
    where
        Layout: Default,
    {
        Self {
            data,
            extents,
            layout: Layout::default(),
        }
    }
    pub fn with_layout(data: &'a [T], extents: (usize, usize), layout: Layout) -> Self {
        Self {
            data,
            extents,
            layout,
        }
    }
}

impl<T, Layout: MatrixLayout> Index<(usize, usize)> for MatrixView<'_, T, Layout> {
    type Output = T;

    fn index(&self, index: (usize, usize)) -> &Self::Output {
        &self.data[self.layout.map_indices(self.extents, index)]
    }
}

pub struct MatrixViewMut<'a, T, Layout: MatrixLayout = LayoutRight> {
    extents: (usize, usize),
    data: &'a mut [T],
    layout: Layout,
}

impl<T, Layout: MatrixLayout> MatrixViewMut<'_, T, Layout> {
    pub fn as_view(&self) -> MatrixView<T, Layout> {
        MatrixView {
            data: self.data,
            extents: self.extents,
            layout: self.layout.clone(),
        }
    }
}

impl<'a, T, Layout: MatrixLayout> Index<(usize, usize)> for MatrixViewMut<'a, T, Layout> {
    type Output = T;

    fn index(&self, index: (usize, usize)) -> &Self::Output {
        &self.data[self.layout.map_indices(self.extents, index)]
    }
}

impl<T, Layout: MatrixLayout> Matrix for MatrixView<'_, T, Layout> {
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

impl<T, Layout: MatrixLayout> IndexMut<(usize, usize)> for MatrixViewMut<'_, T, Layout> {
    fn index_mut(&mut self, index: (usize, usize)) -> &mut Self::Output {
        &mut self.data[self.extents.1 * index.0 + index.1]
    }
}

impl<T, Layout: MatrixLayout> Matrix for MatrixViewMut<'_, T, Layout> {
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

impl<T, Layout: MatrixLayout> MatrixMut for MatrixViewMut<'_, T, Layout> {
    fn get_mut(&mut self, indices: (usize, usize)) -> Option<&mut Self::Item> {
        if self.in_bounds(indices) {
            Some(&mut self[indices])
        } else {
            None
        }
    }
}

pub struct MatrixBuf<T, Layout: MatrixLayout = LayoutRight> {
    vec: Vec<T>,
    extents: (usize, usize),
    layout: Layout,
}

impl<T, Layout: MatrixLayout> MatrixBuf<T, Layout> {
    pub fn new(vec: Vec<T>, extents: (usize, usize)) -> Self
    where
        Layout: Default,
    {
        assert!(vec.len() >= extents.0 * extents.1);
        Self {
            vec,
            extents,
            layout: Layout::default(),
        }
    }

    pub fn with_layout(
        vec: Vec<T>,
        extents: (usize, usize),
        layout: Layout,
    ) -> MatrixBuf<T, Layout> {
        MatrixBuf {
            vec,
            extents,
            layout,
        }
    }

    pub fn spread(t: T, extents: (usize, usize)) -> Self
    where
        T: Clone,
        Layout: Default,
    {
        Self {
            vec: vec![t; extents.0 * extents.1],
            extents: (extents.0, extents.1),
            layout: Layout::default(),
        }
    }

    pub fn spread_with_layout(t: T, extents: (usize, usize), layout: Layout) -> Self
    where
        T: Clone,
    {
        Self {
            vec: vec![t; extents.0 * extents.1],
            extents: (extents.0, extents.1),
            layout,
        }
    }

    pub fn as_view(&self) -> MatrixView<T, Layout> {
        MatrixView {
            data: &self.vec,
            extents: self.extents,
            layout: self.layout.clone(),
        }
    }
    pub fn as_view_mut(&mut self) -> MatrixViewMut<T, Layout> {
        MatrixViewMut {
            data: &mut self.vec,
            extents: self.extents,
            layout: self.layout.clone(),
        }
    }
}

impl<T, Layout: MatrixLayout> From<MatrixBuf<T, Layout>> for Vec<T> {
    fn from(value: MatrixBuf<T, Layout>) -> Self {
        value.vec
    }
}

impl<T: Debug, Layout: MatrixLayout> Debug for MatrixView<'_, T, Layout> {
    fn fmt(&self, f: &mut std::fmt::Formatter<'_>) -> std::fmt::Result {
        write!(
            f,
            "{:?}",
            self.indices()
                .map(|rows| rows.map(|indices| &self[indices]).collect::<Vec::<_>>())
                .collect::<Vec<_>>()
        )
    }
}

#[cfg(test)]
mod tests {
    use super::*;

    #[test]
    fn test_indices() {
        let mat_buf = MatrixBuf::<_>::new(vec![1, 2, 3, 4, 5, 6], (2, 3));
        assert_eq!(
            mat_buf
                .as_view()
                .indices()
                .map(|row| { row.collect::<Vec<_>>() })
                .collect::<Vec<_>>(),
            vec![vec![(0, 0), (0, 1), (0, 2)], vec![(1, 0), (1, 1), (1, 2)]]
        );
    }
}

pub fn offset_by((y, x): (usize, usize), (dy, dx): (isize, isize)) -> Option<(usize, usize)> {
    Some((
        (y as isize + dy).try_into().ok()?,
        (x as isize + dx).try_into().ok()?,
    ))
}

pub fn neightbour_indices(
    matrix: impl Matrix,
    coord: (usize, usize),
) -> impl Iterator<Item = (usize, usize)> {
    [(-1, 0), (0, 1), (1, 0), (0, -1)]
        .iter()
        .filter_map(move |&offset| {
            offset_by(coord, offset).filter(|&new_coord| matrix.in_bounds(new_coord))
        })
}
