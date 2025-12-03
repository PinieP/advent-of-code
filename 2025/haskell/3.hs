module Main where

main = do
  inputs <- readFile "../inputs/3.txt"
  putStrLn "Task1:"
  print $ task1 $ parse inputs
  putStrLn "Task2:"
  print $ task2 $ parse inputs

example = "987654321111111\n811111111111119\n234234234234278\n818181911112111"

splitOn :: (Eq a) => a -> [a] -> [[a]]
splitOn _ [] = []
splitOn c xs = first : splitOn c rest
  where
    (first, rest) = splitFirst c xs

splitFirst :: (Eq a) => a -> [a] -> ([a], [a])
splitFirst c xs = (before, drop 1 after)
  where
    (before, after) = span (/= c) xs

parse = splitOn '\n'

count 0 _ = []
count n xs = maxElem : count (n - 1) (tail $ dropWhile (/= maxElem) xs)
  where
    maxElem = maximum $ take (length xs - n + 1) xs

counts n = sum . map (read . count n)

task1 = counts 2

task2 = counts 12
