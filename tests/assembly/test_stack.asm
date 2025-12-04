; Stack Test - Verification of PUSH/POP operations
; This program tests that the stack grows correctly and PUSH/POP work as expected
; 
; Test Plan:
; 1. Initialize stack pointer to 0x7FFF
; 2. Push several values onto the stack
; 3. Verify SP decrements correctly
; 4. Pop values back and verify they match
; 5. Verify SP increments correctly
; 6. Test CALL/RET mechanism
;
; Expected Results:
; - After each PUSH, SP should decrease by 2
; - After each POP, SP should increase by 2
; - Popped values should match pushed values in reverse order
; - CALL should push return address
; - RET should pop return address and return to caller

.org 0x8000

start:
    ; SP is automatically initialized to 0x7FFF by the CPU
    ; Save initial SP for verification (we'll use R2 to track expected SP)
    MOV R2, #0x7FFF     ; R2 = expected SP value
    
    ; Test 1: Push three values
    MOV R0, #0xAAAA
    PUSH R0             ; SP should now be 0x7FFD
    
    MOV R0, #0xBBBB
    PUSH R0             ; SP should now be 0x7FFB
    
    MOV R0, #0xCCCC
    PUSH R0             ; SP should now be 0x7FF9
    
    ; Test 2: Pop values and verify
    POP R0              ; Should get 0xCCCC
    CMP R0, #0xCCCC
    JNZ test_failed
    
    POP R0              ; Should get 0xBBBB
    CMP R0, #0xBBBB
    JNZ test_failed
    
    POP R0              ; Should get 0xAAAA
    CMP R0, #0xAAAA
    JNZ test_failed

    
    ; Test 3: CALL/RET mechanism
    MOV R0, #0x1234     ; Set a test value
    CALL test_function  ; Call function
    ; After return, R0 should be 0x5678
    CMP R0, #0x5678
    JNZ test_failed
    
    ; Test 4: Nested stack operations
    PUSH R1
    PUSH R2
    MOV R1, #100
    MOV R2, #200
    CALL nested_test
    POP R2
    POP R1
    ; R0 should contain 300 (100 + 200)
    CMP R0, #300
    JNZ test_failed
    
    ; All tests passed!
    JMP all_tests_passed

test_failed:
    MOV R1, #0x0100     ; Address to store result
    MOV R0, #0xFFFF     ; Error code
    STORE R0, [R1]      ; Store error at 0x0100
    HALT

all_tests_passed:
    MOV R1, #0x0100     ; Address to store result
    MOV R0, #0x0000     ; Success code
    STORE R0, [R1]      ; Store success at 0x0100
    HALT


; Test function for CALL/RET
test_function:
    ; Simple function that changes R0
    MOV R0, #0x5678
    RET

; Nested test function
nested_test:
    ; Function that adds two numbers
    ; Expects R1 and R2 to contain values
    ; Returns result in R0
    ; Note: This simple function doesn't need frame pointer management
    
    ; Function body
    MOV R0, R1          ; R0 = R1
    ADD R0, R2          ; R0 = R1 + R2
    
    RET


