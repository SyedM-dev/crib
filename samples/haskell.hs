-- File: haskell.hs
{-# LANGUAGE GADTs, TypeFamilies #-}

module SyntaxTest where

import Data.List (sort)
import qualified Data.Map as Map

-- Simple data type
data Person = Person
  { name :: String
  , age  :: Int
  } deriving (Show, Eq)

-- GADT
data Expr a where
  I :: Int -> Expr Int
  B :: Bool -> Expr Bool
  Add :: Expr Int -> Expr Int -> Expr Int
  Eq :: Expr Int -> Expr Int -> Expr Bool

-- Type class
class Describable a where
  describe :: a -> String

instance Describable Person where
  describe (Person n a) = n ++ " is " ++ show a ++ " years old."

-- Function with pattern matching
sumList :: [Int] -> Int
sumList []     = 0
sumList (x:xs) = x + sumList xs

-- Lambda and higher-order functions
applyTwice :: (a -> a) -> a -> a
applyTwice f x = f (f x)

-- Infix operator
infixl 6 +++
(+++) :: Int -> Int -> Int
a +++ b = a + b

-- IO function
main :: IO ()
main = do
  let people = [Person "Alice" 30, Person "Bob" 25]
  mapM_ (putStrLn . describe) people
  print $ sumList [1..10]
  print $ applyTwice (+1) 5
  print $ 3 +++ 4
  print $ Eq (I 2) (Add (I 1) (I 1))

-- Quasi-quote example
someExpr :: Expr Int
someExpr = [| Add (I 5) (I 7) |]

-- Comments and Haddocks
-- | This is a Haddock comment
-- explaining the module and functions
