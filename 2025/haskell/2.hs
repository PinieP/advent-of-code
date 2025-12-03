module Main where

import Data.List (subsequences)
import Data.List.NonEmpty (groupBy1)
import Data.Maybe

main = do
  inputs <- readFile "../inputs/2.txt"
  print "Task1:\n"
  print $ task1 (parse inputs)
  print "Task2:\n"
  print $ task2 (parse inputs)

splitOn :: (Eq a) => a -> [a] -> [[a]]
splitOn _ [] = []
splitOn c xs = first : splitOn c rest
  where
    (first, rest) = splitFirst c xs

splitFirst :: (Eq a) => a -> [a] -> ([a], [a])
splitFirst c xs = (before, drop 1 after)
  where
    (before, after) = span (/= c) xs

example = "11-22,95-115,998-1012,1188511880-1188511890,222220-222224,1698522-1698528,446443-446449,38593856-38593862,565653-565659,824824821-824824827,2121212118-2121212124"

parseToInts :: String -> [Integer]
parseToInts = concatMap (uncurry enumFromTo . both read . splitFirst '-') . splitOn ','
  where
    both f (x, y) = (f x, f y)

parse = map show . parseToInts

isInvalid :: (Eq a) => [a] -> Bool
isInvalid xs = uncurry (==) (splitAt (length xs `div` 2) xs)

task1 = sum . map read . filter isInvalid

chunkN _ [] = []
chunkN n xs = first : chunkN n rest where (first, rest) = splitAt n xs

subchunks xs = map (`chunkN` xs) $ filter ((== 0) . rem (length xs)) [1 .. length xs - 1]

allEqual xs = all (== head xs) (tail xs)

invalidIds :: [String] -> [String]
invalidIds = mapMaybe f
  where
    f num = if any allEqual (subchunks num) then Just num else Nothing

task2 = sum . map read . invalidIds
