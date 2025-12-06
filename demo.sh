#!/bin/bash

# Software CPU Project - Complete Demo Script
# Run this script to build, test, and launch the interactive trace viewer

set -e  # Exit on any error

# Colors for output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
NC='\033[0m' # No Color

echo -e "${BLUE} Software CPU Project - Interactive Demo${NC}"
echo -e "${BLUE}===========================================${NC}"
echo ""

# Check if we're in the right directory
if [ ! -f "Makefile" ] || [ ! -d "src" ]; then
    echo -e "${RED} Error: Please run this script from the software-cpu project root directory${NC}"
    echo "Expected files: Makefile, src/, trace_viewer/"
    exit 1
fi

# Step 1: Build the project
echo -e "${YELLOW} Step 1: Building the project...${NC}"
if make all; then
    echo -e "${GREEN} Build successful!${NC}"
else
    echo -e "${RED} Build failed. Please check the error messages above.${NC}"
    exit 1
fi
echo ""

# Step 2: Run unit tests
echo -e "${YELLOW} Step 2: Running unit tests...${NC}"
if make test; then
    echo -e "${GREEN} Unit tests passed!${NC}"
else
    echo -e "${RED}  Unit tests failed, but continuing with demo...${NC}"
fi
echo ""

# Step 3: Generate execution traces
echo -e "${YELLOW} Step 3: Generating execution traces...${NC}"

# Create traces directory if it doesn't exist
mkdir -p build/traces
# Clean up old traces to prevent duplicates
rm -f build/traces/*.json build/traces/*.map.json

# Generate traces for different programs

echo "  → Creating Fibonacci sequence trace..."
./scripts/run_general_with_trace.sh src/programs/fibonacci.asm Fibonacci_sequence.json

echo "  → Creating Hello World trace..."
./scripts/run_general_with_trace.sh src/programs/hello_world.asm Hello_world.json

echo "  → Creating Timer example trace..."
./scripts/run_general_with_trace.sh src/programs/timer_example.asm Timer_example.json

echo "  → Creating Factorial (recursive) trace..."
./scripts/run_general_with_trace.sh src/programs/factorial.asm Factorial_recursive.json

echo "  → Creating integration test trace..."
./scripts/run_general_with_trace.sh tests/assembly/test_integration.asm Integration_test.json

echo "  → Creating comprehensive instruction test trace..."
./scripts/run_general_with_trace.sh tests/assembly/test_phase4b.asm Comprehensive_instruction_test.json

echo "  → Creating branch flags trace..."
./scripts/run_general_with_trace.sh tests/assembly/test_branch_flags.asm Branch_flags_test.json

echo -e "${GREEN} All traces generated successfully!${NC}"
echo ""

# Generate trace manifest
echo -e "${YELLOW} Generating trace manifest...${NC}"
cat <<EOF > build/traces/traces.json
[
$(ls build/traces/*.json | grep -v "traces.json" | grep -v ".map.json" | xargs -n 1 basename | sed 's/^/"/;s/$/",/' | sed '$s/,$//')
]
EOF

# Step 6: Launch trace viewer
echo -e "${YELLOW} Step 5: Launching interactive trace viewer...${NC}"
echo ""
echo -e "${GREEN} DEMO INSTRUCTIONS:${NC}"
echo -e "${GREEN}===================${NC}"
echo "1. Your browser will open to the CPU Trace Viewer"
echo "2. Select a trace from the list to visualize it"
echo "3. Use the slider to step through execution cycles"
echo "4. Watch registers and instructions change in real-time"
echo "5. Press Ctrl+C in this terminal to stop the demo"
echo ""
echo -e "${YELLOW} Starting web server...${NC}"

# Check if Python 3 is available
if command -v python3 &> /dev/null; then
    PYTHON_CMD="python3 -m http.server 8000"
elif command -v python &> /dev/null; then
    PYTHON_CMD="python -m SimpleHTTPServer 8000"
else
    echo -e "${RED} Error: Python not found. Please install Python to run the web server.${NC}"
    echo "Alternatively, you can manually serve the project root with any web server."
    exit 1
fi

# Serve from project root
# cd trace_viewer  <-- Removed this line

# Try to open browser automatically
if command -v open &> /dev/null; then
    # macOS
    sleep 2 && open http://localhost:8000/trace_viewer/ &
elif command -v xdg-open &> /dev/null; then
    # Linux
    sleep 2 && xdg-open http://localhost:8000/trace_viewer/ &
elif command -v start &> /dev/null; then
    # Windows
    sleep 2 && start http://localhost:8000/trace_viewer/ &
fi

echo ""
echo -e "${GREEN} Trace viewer running at: http://localhost:8000/trace_viewer/${NC}"
echo -e "${GREEN} Trace files location: build/traces/${NC}"
echo ""
echo -e "${YELLOW}Press Ctrl+C to stop the demo${NC}"
echo ""

# Start the web server
$PYTHON_CMD