; ModuleID = 'crt1.ll'
source_filename = "vapor/runtime/crt1.ll"
target triple = "x86_64-pc-linux-gnu"

; Function Attrs: nounwind readonly
declare i32 @atoi(i8* nocapture) local_unnamed_addr #0

; Function Attrs: nounwind
declare i32 @__entry_call_thunk(i32) local_unnamed_addr #1

; Function Attrs: nounwind
define i32 @main(i32 %argc, i8** nocapture readonly %argv) local_unnamed_addr #1 {
entry:
  %0 = getelementptr i8*, i8** %argv, i64 1
  %1 = load i8*, i8** %0, align 8
  %2 = tail call i32 @atoi(i8* %1)
  %3 = tail call i32 @__entry_call_thunk(i32 %2)
  ret i32 %3
}

attributes #0 = { nounwind readonly }
attributes #1 = { nounwind }
