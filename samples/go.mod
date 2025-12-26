module github.com/example/project

go 1.21

// ==============================
// Direct dependencies
// ==============================
require (
    github.com/sirupsen/logrus v1.10.0
    golang.org/x/net v0.10.0
    github.com/pkg/errors v0.9.2 // indirect
)

// ==============================
// Replace dependencies
// ==============================
replace (
    github.com/old/dependency v1.2.3 => github.com/new/dependency v1.2.4
    golang.org/x/oldnet => golang.org/x/net v0.11.0
)

// ==============================
// Exclude dependencies
// ==============================
exclude github.com/bad/dependency v1.0.0

// ==============================
// Indirect dependencies
// ==============================
require (
    github.com/another/pkg v1.3.0 // indirect
)
