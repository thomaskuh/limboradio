import { Injectable } from '@angular/core';
import {HttpClient} from '@angular/common/http';
import {RadioResponse} from './radio-response';
import {Observable, throwError} from 'rxjs';
import {RadioRequest} from './radio-request';
import {catchError} from 'rxjs/internal/operators';
import {MatDialog} from '@angular/material';
import {DialogErrorComponent} from '../dialog-error/dialog-error.component';

@Injectable({
  providedIn: 'root'
})
export class ClientService {

  constructor(private http: HttpClient, private dialog: MatDialog) {
  }

  getState(): Observable<RadioResponse> {
    return this.http.get<RadioResponse>('/api/state');
  }

  getStreams(): Observable<RadioResponse> {
    return this.http.get<RadioResponse>('/api/streams');
  }

  saveThatThang(state: RadioRequest): Observable<void> {
    return this.http.post<void>('/api/update', state).pipe(
      catchError(
        err => {
          this.dialog.open(DialogErrorComponent, {data: err.error});
          return throwError(err);
        })
    );
  }

}
