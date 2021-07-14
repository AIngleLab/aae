" Vim syntax file
" Language: AIngle IDL
" Maintainer: Daniel Lundin <dln@eintr.org>
" Last Change: 20100924
" Copy to ~/.vim/syntax/
" Add to ~/.vimrc
"  au BufRead,BufNewFile *.avdl setlocal filetype=aingle-idl
"
" Licensed to the Apache Software Foundation (ASF) under one
" or more contributor license agreements. See the NOTICE file
" distributed with this work for additional information
" regarding copyright ownership. The ASF licenses this file
" to you under the Apache License, Version 2.0 (the
" "License"); you may not use this file except in compliance
" with the License. You may obtain a copy of the License at
"
"   
"
" Unless required by applicable law or agreed to in writing,
" software distributed under the License is distributed on an
" "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
" KIND, either express or implied. See the License for the
" specific language governing permissions and limitations
" under the License.
"

if version < 600
  syntax clear
elseif exists("b:current_syntax")
  finish
endif

" Todo
syn keyword aingleTodo TODO todo FIXME fixme XXX xxx contained

" Comments
syn region aingleComment start="/\*" end="\*/" contains=aingleTodo
syn match aingleComment "//.\{-}\(?>\|$\)\@=" contains=aingleTodo

" Identifiers
syn region aingleIdentifier start="^\s*\(error\|protocol\|record\)" end="{" contains=aingleIdentifierType 
syn keyword aingleIdentifierType error protocol record contained nextgroup=aingleIdentifierName skipwhite
syn match aingleIdentifierName	"\w\w*" display contained skipwhite

syn region aingleEscaped  start=/`/ end=/`/

" Types
syn match aingleNumber "-\=\<\d\+\>" contained
syn region aingleString start=/"/ skip=/\\"/ end=/"/
syn region aingleString start=/'/ skip=/\\'/ end=/'/
syn region aingleArray  start="<" end=">" contains=aingleArrayType
syn match aingleArrayType "\w\w*" display contained skipwhite

" Keywords
syn keyword aingleKeyword java-class namespace order
syn keyword aingleKeyword error throws
syn keyword aingleBasicTypes boolean bytes double fixed float int long null string void
syn keyword aingleStructure array enum map union

if version >= 508 || !exists("did_aingle_idl_syn_inits")
  if version < 508
    let did_aingle_idl_syn_inits = 1
    command! -nargs=+ HiLink hi link <args>
  else
    command! -nargs=+ HiLink hi def link <args>
  endif

  HiLink aingleTodo Todo
  HiLink aingleComment Comment
  HiLink aingleNumber Number
  HiLink aingleKeyword Define
  HiLink aingleIdentifierType Special
  HiLink aingleBasicTypes Type
  HiLink aingleArrayType Type
  HiLink aingleString       String
  HiLink aingleStructure Structure
  HiLink aingleArray Structure
  HiLink aingleEscaped Default
  HiLink aingleIdentifierName    Entity

  delcommand HiLink
endif

let b:current_syntax = "aingle-idl"
