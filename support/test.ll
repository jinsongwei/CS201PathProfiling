; ModuleID = 'support/test.bc'
target datalayout = "e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-pc-linux-gnu"

@.str = private unnamed_addr constant [12 x i8] c"Hello World\00", align 1
@hw = global i8* getelementptr inbounds ([12 x i8]* @.str, i32 0, i32 0), align 8
@y = global i32 100, align 4
@.str1 = private unnamed_addr constant [17 x i8] c"Enter a number: \00", align 1
@.str2 = private unnamed_addr constant [3 x i8] c"%d\00", align 1
@.str3 = private unnamed_addr constant [17 x i8] c"You entered: %d\0A\00", align 1
@.str4 = private unnamed_addr constant [10 x i8] c"%s %d %u\0A\00", align 1
@.str5 = private unnamed_addr constant [10 x i8] c"%d %s %u\0A\00", align 1
@.str6 = private unnamed_addr constant [8 x i8] c"j: %d \0A\00", align 1
@bbCounter = internal global i32 0
@BasicBlockPrintfFormatStr = private constant [14 x i8] c"BB Count: %d\0A\00"

; Function Attrs: nounwind uwtable
define i32 @main() #0 {
  %1 = load i32* @bbCounter
  %2 = add i32 1, %1
  store i32 %2, i32* @bbCounter
  %3 = alloca i32, align 4
  %x = alloca i32, align 4
  %j = alloca i32, align 4
  %i = alloca i32, align 4
  store i32 0, i32* %3
  store i32 0, i32* %x, align 4
  %4 = call i32 (i8*, ...)* @printf(i8* getelementptr inbounds ([17 x i8]* @.str1, i32 0, i32 0))
  %5 = call i32 (i8*, ...)* @__isoc99_scanf(i8* getelementptr inbounds ([3 x i8]* @.str2, i32 0, i32 0), i32* %x)
  %6 = load i32* %x, align 4
  %7 = call i32 (i8*, ...)* @printf(i8* getelementptr inbounds ([17 x i8]* @.str3, i32 0, i32 0), i32 %6)
  %8 = load i32* %x, align 4
  %9 = load i32* @y, align 4
  %10 = icmp ugt i32 %8, %9
  br i1 %10, label %11, label %18

; <label>:11                                      ; preds = %0
  %12 = load i32* @bbCounter
  %13 = add i32 1, %12
  store i32 %13, i32* @bbCounter
  %14 = load i8** @hw, align 8
  %15 = load i32* %x, align 4
  %16 = load i32* @y, align 4
  %17 = call i32 (i8*, ...)* @printf(i8* getelementptr inbounds ([10 x i8]* @.str4, i32 0, i32 0), i8* %14, i32 %15, i32 %16)
  br label %25

; <label>:18                                      ; preds = %0
  %19 = load i32* @bbCounter
  %20 = add i32 1, %19
  store i32 %20, i32* @bbCounter
  %21 = load i32* %x, align 4
  %22 = load i8** @hw, align 8
  %23 = load i32* @y, align 4
  %24 = call i32 (i8*, ...)* @printf(i8* getelementptr inbounds ([10 x i8]* @.str5, i32 0, i32 0), i32 %21, i8* %22, i32 %23)
  br label %25

; <label>:25                                      ; preds = %18, %11
  %26 = load i32* @bbCounter
  %27 = add i32 1, %26
  store i32 %27, i32* @bbCounter
  store i32 0, i32* %j, align 4
  store i32 0, i32* %i, align 4
  br label %28

; <label>:28                                      ; preds = %40, %25
  %29 = load i32* @bbCounter
  %30 = add i32 1, %29
  store i32 %30, i32* @bbCounter
  %31 = load i32* %i, align 4
  %32 = load i32* %x, align 4
  %33 = icmp slt i32 %31, %32
  br i1 %33, label %34, label %45

; <label>:34                                      ; preds = %28
  %35 = load i32* @bbCounter
  %36 = add i32 1, %35
  store i32 %36, i32* @bbCounter
  %37 = load i32* %x, align 4
  %38 = load i32* %j, align 4
  %39 = add nsw i32 %38, %37
  store i32 %39, i32* %j, align 4
  br label %40

; <label>:40                                      ; preds = %34
  %41 = load i32* @bbCounter
  %42 = add i32 1, %41
  store i32 %42, i32* @bbCounter
  %43 = load i32* %i, align 4
  %44 = add nsw i32 %43, 1
  store i32 %44, i32* %i, align 4
  br label %28

; <label>:45                                      ; preds = %28
  %46 = load i32* @bbCounter
  %47 = add i32 1, %46
  store i32 %47, i32* @bbCounter
  %48 = load i32* %j, align 4
  %49 = call i32 (i8*, ...)* @printf(i8* getelementptr inbounds ([8 x i8]* @.str6, i32 0, i32 0), i32 %48)
  %50 = load i32* @bbCounter
  %51 = call i32 (i8*, ...)* @printf(i8* getelementptr inbounds ([14 x i8]* @BasicBlockPrintfFormatStr, i32 0, i32 0), i32 %50)
  ret i32 0
}

declare i32 @printf(i8*, ...) #1

declare i32 @__isoc99_scanf(i8*, ...) #1

attributes #0 = { nounwind uwtable "less-precise-fpmad"="false" "no-frame-pointer-elim"="true" "no-frame-pointer-elim-non-leaf" "no-infs-fp-math"="false" "no-nans-fp-math"="false" "stack-protector-buffer-size"="8" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #1 = { "less-precise-fpmad"="false" "no-frame-pointer-elim"="true" "no-frame-pointer-elim-non-leaf" "no-infs-fp-math"="false" "no-nans-fp-math"="false" "stack-protector-buffer-size"="8" "unsafe-fp-math"="false" "use-soft-float"="false" }

!llvm.ident = !{!0}

!0 = !{!"Ubuntu clang version 3.4-1ubuntu3 (tags/RELEASE_34/final) (based on LLVM 3.4)"}
