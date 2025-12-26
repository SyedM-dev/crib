// file: go.go
package example_test

import (
	"context"
	"fmt"
	"math"
	"sync"
	"testing"
	"time"
)

// Simple interface
type Adder interface {
	Add(a, b int) int
}

// Concrete implementation
type Calculator struct{}

func (Calculator) Add(a, b int) int {
	return a + b
}

// Generic helper
func Max[T ~int | ~float64](a, b T) T {
	if a > b {
		return a
	}
	return b
}

// Table-driven test
func TestAdd(t *testing.T) {
	tests := []struct {
		name     string
		a, b     int
		expected int
	}{
		{"positive", 2, 3, 5},
		{"negative", -2, -3, -5},
		{"mixed", -2, 5, 3},
	}

	var calc Adder = Calculator{}

	for _, tt := range tests {
		t.Run(tt.name, func(t *testing.T) {
			if got := calc.Add(tt.a, tt.b); got != tt.expected {
				t.Fatalf("Add(%d, %d) = %d; want %d",
					tt.a, tt.b, got, tt.expected)
			}
		})
	}
}

// Concurrency + context test
func TestWorker(t *testing.T) {
	ctx, cancel := context.WithTimeout(context.Background(), 100*time.Millisecond)
	defer cancel()

	ch := make(chan int)
	var wg sync.WaitGroup

	wg.Add(1)
	go func() {
		defer wg.Done()
		select {
		case ch <- 42:
		case <-ctx.Done():
		}
	}()

	select {
	case v := <-ch:
		if v != 42 {
			t.Errorf("unexpected value: %d", v)
		}
	case <-ctx.Done():
		t.Fatal("timed out")
	}

	wg.Wait()
}

// Raw string + math edge case
func TestRawString(t *testing.T) {
	raw := `line 1
line 2
\t not escaped
`
	if len(raw) == 0 || math.IsNaN(float64(len(raw))) {
		t.Fatal("impossible condition reached")
	}
}
