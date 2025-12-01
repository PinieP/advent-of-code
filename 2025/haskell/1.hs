module Main where

import Data.Char (isDigit)

main = do
  inputs <- readFile "../inputs/1.txt"
  print $ task1' $ parse inputs
  print $ task2 $ parse inputs

example = "L68\nL30\nR48\nL5\nR60\nL55\nL1\nL99\nR14\nL82"

parse [] = []
parse (s : rest) = (sign * read digits) : parse (drop 1 end)
  where
    (digits, end) = span isDigit rest
    sign = case s of
      'L' -> -1
      'R' -> 1

scan = scanl nextState
  where
    nextState st num = mod (st + num) 100

task1' :: [Int] -> Int
task1' = fst . foldl f (0, 50)
  where
    f (count, st) num = (count + fromEnum (state == 0), state)
      where
        state = mod (st + num) 100

task1 = length . filter (0 ==) . scan 50

task2 = length . concatMap (filter (0 ==)) . scanl f [50]
  where
    f st num = tail $ scan (last st) $ replicate (abs num) (signum num)
