; ModuleID = 'support/1.bc'
target datalayout = "e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-pc-linux-gnu"

@bbCounter = internal global i32 0
@BasicBlockPrintfFormatStr = private constant [14 x i8] c"BB Count: %d\0A\00"

; Function Attrs: nounwind uwtable
define void @function_1(i32 %x) #0 {
"0":
  %0 = load i32* @bbCounter
  %1 = add i32 1, %0
  store i32 %1, i32* @bbCounter
  %2 = alloca i32, align 4
  store i32 %x, i32* %2, align 4
  br label %"1"

"1":                                              ; preds = %"6", %"0"
  %3 = load i32* @bbCounter
  %4 = add i32 1, %3
  store i32 %4, i32* @bbCounter
  %5 = load i32* %2, align 4
  %6 = icmp ugt i32 %5, 0
  br i1 %6, label %"3", label %"2"

"2":                                              ; preds = %"1"
  %7 = load i32* @bbCounter
  %8 = add i32 1, %7
  store i32 %8, i32* @bbCounter
  br label %"7"

"3":                                              ; preds = %"1"
  %9 = load i32* @bbCounter
  %10 = add i32 1, %9
  store i32 %10, i32* @bbCounter
  %11 = load i32* %2, align 4
  %12 = urem i32 %11, 4
  %13 = icmp eq i32 %12, 0
  br i1 %13, label %"4", label %"5"

"4":                                              ; preds = %"3"
  %14 = load i32* @bbCounter
  %15 = add i32 1, %14
  store i32 %15, i32* @bbCounter
  %16 = load i32* %2, align 4
  %17 = add i32 %16, -1
  store i32 %17, i32* %2, align 4
  br label %"6"

"5":                                              ; preds = %"3"
  %18 = load i32* @bbCounter
  %19 = add i32 1, %18
  store i32 %19, i32* @bbCounter
  %20 = load i32* %2, align 4
  %21 = add i32 %20, -1
  store i32 %21, i32* %2, align 4
  br label %"6"

"6":                                              ; preds = %"5", %"4"
  %22 = load i32* @bbCounter
  %23 = add i32 1, %22
  store i32 %23, i32* @bbCounter
  br label %"1"

"7":                                              ; preds = %"2"
  %24 = load i32* @bbCounter
  %25 = add i32 1, %24
  store i32 %25, i32* @bbCounter
  ret void
}

; Function Attrs: nounwind uwtable
define i32 @main() #0 {
"0":
  %0 = load i32* @bbCounter
  %1 = add i32 1, %0
  store i32 %1, i32* @bbCounter
  %2 = alloca i32, align 4
  store i32 0, i32* %2
  call void @function_1(i32 100)
  %3 = load i32* @bbCounter
  %4 = call i32 (i8*, ...)* @printf(i8* getelementptr inbounds ([14 x i8]* @BasicBlockPrintfFormatStr, i32 0, i32 0), i32 %3)
  ret i32 0
}

declare i32 @printf(i8*, ...)

attributes #0 = { nounwind uwtable "less-precise-fpmad"="false" "no-frame-pointer-elim"="true" "no-frame-pointer-elim-non-leaf" "no-infs-fp-math"="false" "no-nans-fp-math"="false" "stack-protector-buffer-size"="8" "unsafe-fp-math"="false" "use-soft-float"="false" }

!llvm.ident = !{!0}

!0 = !{!"Ubuntu clang version 3.4-1ubuntu3 (tags/RELEASE_34/final) (based on LLVM 3.4)"}
